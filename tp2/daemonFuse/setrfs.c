/*
 * Squelette d'implementation du système de fichier setrFS
 * Adapté de l'exemple "passthrough" de libfuse : https://github.com/libfuse/libfuse/blob/master/example/passthrough.c
 * Distribué sous licence GPL
 *
 *
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

// Sockets UNIX
#include <sys/socket.h>
#include <sys/un.h>

// pthread pour les mutex
#include <pthread.h>

#include "communications.h"

#include "fstools.h"

const char unixSockPath[] = "/tmp/unixsocket";

// Cette fonction initialise le cache et l'insère dans le contexte de FUSE, qui sera
// accessible à toutes les autres fonctions.
// Elle est déjà implémentée pour vous, mais vous pouvez la modifier au besoin.
void *setrfs_init(struct fuse_conn_info *conn) {
    struct cacheData cache;
    cache.rootDirIndex = NULL;
    cache.firstFile = NULL;
    pthread_mutex_init(&(cache.mutex), NULL);
    char *cachePtr = malloc(sizeof cache);
    memcpy(cachePtr, &cache, sizeof cache);

    return (void *) cachePtr;
}


static int setrfs_getattr(const char *path, struct stat *stbuf) {
    printf("getattr: %s\n", path);
    // On récupère le contexte
    struct fuse_context *context = fuse_get_context();
    struct cacheData* cache = (cacheData *) context->private_data;

    // Si vous avez enregistré dans données dans setrfs_init, alors elles sont disponibles dans context->private_data
    // Ici, voici un exemple où nous les utilisons pour donner le bon propriétaire au fichier (l'utilisateur courant)
    stbuf->st_uid = context->uid;        // On indique l'utilisateur actuel comme proprietaire
    stbuf->st_gid = context->gid;        // Idem pour le groupe

    stbuf->st_dev = 0;
    stbuf->st_ino = 0;
    stbuf->st_nlink = 1;
    stbuf->st_rdev = 0;
    stbuf->st_blksize = 0;
    stbuf->st_blocks = 0;

    // on determine la taille du fichier
    pthread_mutex_lock(&cache->mutex);
    cacheFichier* file = trouverFichierEnCache(path, cache);
    size_t len = 1;
    if (file != NULL) {
        len = file->len;
    }
    stbuf->st_mode = 0777;
    if (strcmp(path, "/") == 0) {
        stbuf->st_nlink = 2;
        stbuf->st_size = 1;
        stbuf->st_mode |= S_IFDIR;
    } else {
        stbuf->st_size = len;
        stbuf->st_mode |= S_IFREG;
    }
    pthread_mutex_unlock(&cache->mutex);

    return 0;
}


// Cette fonction est utilisée pour lister un dossier. Elle est déjà implémentée pour vous
static int setrfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    printf("setrfs_readdir : %s\n", path);
    (void) offset;
    (void) fi;

    struct fuse_context *context = fuse_get_context();
    struct cacheData *cache = (struct cacheData *) context->private_data;
    if (cache->rootDirIndex == NULL) {
        // Le listing du repertoire n'est pas en cache
        // On doit faire une requete
        // On ouvre un socket
        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == -1) {
            perror("Impossible d'initialiser le socket UNIX");
            return -1;
        }

        // Ecriture des parametres du socket
        struct sockaddr_un sockInfo;
        memset(&sockInfo, 0, sizeof sockInfo);
        sockInfo.sun_family = AF_UNIX;
        strncpy(sockInfo.sun_path, unixSockPath, sizeof sockInfo.sun_path - 1);

        printf("Gonna open socket\n");

        // Connexion
        if (connect(sock, (const struct sockaddr *) &sockInfo, sizeof sockInfo) < 0) {
            perror("Erreur connect");
            exit(1);
        }

        printf("Socket opened\n");
        // Formatage et envoi de la requete
        //size_t len = strlen() + 1;		// +1 pour le caractere NULL de fin de chaine
        struct msgReq req;
        req.type = REQ_LIST;
        req.sizePayload = 0;
        int octetsTraites = envoyerMessage(sock, &req, NULL);

        // On attend et on recoit le fichier demande
        struct msgRep rep;
        printf("Gonna read socket\n");
        octetsTraites = read(sock, &rep, sizeof rep);
        if (octetsTraites == -1) {
            perror("Erreur en effectuant un read() sur un socket pret");
            exit(1);
        }
        if (VERBOSE)
            printf("Lecture de l'en-tete de la reponse sur le socket %i\n", sock);
        printf("Socket read\n");

        printf("gonna get lock now\n");
        pthread_mutex_lock(&(cache->mutex));
        cache->rootDirIndex = malloc(rep.sizePayload + 1);
        cache->rootDirIndex[rep.sizePayload] = 0;        // On s'assure d'avoir le caractere nul a la fin de la chaine
        unsigned int totalRecu = 0;
        // Il se peut qu'on ait a faire plusieurs lectures si le fichier est gros
        while (totalRecu < rep.sizePayload) {
            octetsTraites = read(sock, cache->rootDirIndex + totalRecu, rep.sizePayload - totalRecu);
            totalRecu += octetsTraites;
        }
        pthread_mutex_unlock(&(cache->mutex));
    }

    // On va utiliser strtok, qui modifie la string
    // On utilise donc une copie
    char *indexStr = malloc(strlen(cache->rootDirIndex) + 1);
    strcpy(indexStr, cache->rootDirIndex);

    // FUSE s'occupe deja des pseudo-fichiers "." et "..",
    // donc on se contente de lister le fichier d'index qu'on vient de recevoir

    char *nomFichier = strtok(indexStr, "\n");        // On assume des fins de lignes UNIX
    int countInode = 1;
    while (nomFichier != NULL) {
        struct stat st;
        memset(&st, 0, sizeof st);
        st.st_ino = 1;            // countInode++;
        st.st_mode = (S_IFREG & S_IFMT) | 0777;    // Fichier regulier, permissions 777
        //if(VERBOSE)
        //	printf("Insertion du fichier %s dans la liste du repertoire\n", nomFichier);
        if (filler(buf, nomFichier, &st, 0)) {
            perror("Erreur lors de l'insertion du fichier dans la liste du repertoire!");
            break;
        }
        nomFichier = strtok(NULL, "\n");
    }
    free(nomFichier);

    return 0;
}


static int setrfs_open(const char *path, struct fuse_file_info *fi) {

    struct fuse_context *context = fuse_get_context();
    cacheData *cache = (cacheData *) context->private_data;

    // 0 si le path est pas dans l'index
    if (checkPathExistence(path, cache) == 0)
    {
        errno = EACCES;
        return -1;
    }

    printf("\nopen(%s)\n", path);

    // section critique
    pthread_mutex_lock(&cache->mutex);
    struct cacheFichier *fhCache = trouverFichierEnCache(path, cache);
    pthread_mutex_unlock(&cache->mutex);

    // on ouvre un socket
    int sockfd;
    if (ouvrirSocket(&sockfd, unixSockPath) == -1) {
        printf("Erreur pour ouvrir le socket a l'ouverture d'un fichier.");
        return -1;
    }

    msgRep rep;
    if (fhCache == NULL) {
        printf("mise en cache de (%s)\n", path);
        // on essaye d'obtenir le fichier de l'ajouter a la cache
        size_t pathLen = strlen(path);
        msgReq header = {REQ_READ, pathLen};
        envoyerMessage(sockfd, &header, NULL);
        write(sockfd, path, pathLen);
        int bytesRead = read(sockfd, &rep, sizeof(msgRep));
        if (bytesRead == -1) {
            perror("Erreur lecture sur socket pret.");
            exit(1);
        }
        if (rep.status == STATUS_ERREUR_TELECHARGEMENT) {
            printf("Erreur telechargement.\n");
            return -1;
        }

        cacheFichier* file = malloc(sizeof(cacheFichier));
        memset(file, 0, sizeof(cacheFichier));

        char* name = malloc(pathLen*sizeof(char) + 1);
        file->nom = name;
        strncpy(file->nom, path, pathLen);
        file->nom[pathLen] = '\0';
        // lecture du contenu
        size_t bytesToRead = rep.sizePayload;
        char* content = malloc(bytesToRead + 1);
        content[bytesToRead] = NULL;
        file->data = content;
        size_t byteProcessed = 0;
        while (byteProcessed < bytesToRead) {
            byteProcessed += read(sockfd, file->data + byteProcessed, bytesToRead - byteProcessed);
        }

        // ecriture des props du fichier dans la structure et insertion dans la cache
        file->len = bytesToRead;
        file->countOpen = 1;
        file->prev = NULL;
        file->next = NULL;

        // section critique pour manipuler la cache
        pthread_mutex_lock(&cache->mutex);
        insererFichier(file, cache);
        pthread_mutex_unlock(&cache->mutex);

        fi->fh = file;
        return 0;
    }
    else {
        // le fichier est deja en cache, on se contente de le retourner
        fi->fh = fhCache;
        fhCache->countOpen++;
    }
    // nous ne devrions pas frapper cette ligne logiquement
    return -1;
}


static int setrfs_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    printf("\nread: %s\n", path);
    cacheFichier* file = fi->fh;

    // correction de la taille de lecture
    size_t fileSize = file->len;
    if (size > fileSize - offset) {
        size = fileSize - offset;
    }

    memcpy(buf, file->data + offset, size);

    return size;
}


static int setrfs_release(const char *path, struct fuse_file_info *fi) {
    printf("\nrelease: %s\n", path);
    struct fuse_context *context = fuse_get_context();
    cacheData *cache = (cacheData *) context->private_data;

    pthread_mutex_lock(&cache->mutex);
    cacheFichier *file = fi->fh;
    file->countOpen--;
    if (file->countOpen <= 0) {
        retireFichier(file, cache);
    }
    pthread_mutex_unlock(&cache->mutex);

    return 0;
}


///////////////////////////////////////
// Plus rien à faire à partir d'ici! //
// Les fonctions restantes n'ont pas //
// à être implémentées pour que le   //
// système de fichiers fonctionne.   //
///////////////////////////////////////

static int setrfs_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_write\n");

    return 0;
}


static int setrfs_access(const char *path, int mask) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_access for path %s\n", path);
    return 0;
}

static int setrfs_readlink(const char *path, char *buf, size_t size) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_readlink\n");
    return 0;
}

static int setrfs_statfs(const char *path, struct statvfs *stbuf) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_statfs\n");

    return 0;
}


static int setrfs_mknod(const char *path, mode_t mode, dev_t rdev) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_mknod\n");

    return 0;
}

static int setrfs_mkdir(const char *path, mode_t mode) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_mkdir\n");


    return 0;
}

static int setrfs_unlink(const char *path) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_unlink\n");

    return 0;
}

static int setrfs_rmdir(const char *path) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_rmdir\n");


    return 0;
}

static int setrfs_symlink(const char *from, const char *to) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_symlink\n");

    return 0;
}

static int setrfs_rename(const char *from, const char *to) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_rename\n");

    return 0;
}

static int setrfs_link(const char *from, const char *to) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_link\n");

    return 0;
}

static int setrfs_chmod(const char *path, mode_t mode) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_chmod\n");

    return 0;
}

static int setrfs_chown(const char *path, uid_t uid, gid_t gid) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_chown\n");

    return 0;
}

static int setrfs_truncate(const char *path, off_t size) {
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_truncate\n");

    return 0;
}

#ifdef HAVE_UTIMENSAT
static int setrfs_utimens(const char *path, const struct timespec ts[2])
{
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_utimens\n");

    return 0;
}
#endif

static int setrfs_fsync(const char *path, int isdatasync,
                        struct fuse_file_info *fi) {
    /* Just a stub.	 This method is optional and can safely be left
       unimplemented */

    (void) path;
    (void) isdatasync;
    (void) fi;
    printf("setrfs_fsync\n");
    printf("#### NOT IMPLEMENTED! ####");
    return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int setrfs_fallocate(const char *path, int mode,
            off_t offset, off_t length, struct fuse_file_info *fi)
{
    int fd;
    int res;
    printf("#### NOT IMPLEMENTED! ####");
    printf("setrfs_fallocate\n");
    return 0
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int setrfs_setxattr(const char *path, const char *name, const char *value,
            size_t size, int flags)
{
printf("#### NOT IMPLEMENTED! ####");
printf("setrfs_setxattr\n");

    return 0;
}

static int setrfs_getxattr(const char *path, const char *name, char *value,
            size_t size)
{
printf("#### NOT IMPLEMENTED! ####");
printf("setrfs_getxattr\n");

    return res;
}

static int setrfs_listxattr(const char *path, char *list, size_t size)
{
printf("#### NOT IMPLEMENTED! ####");
printf("setrfs_listxattr\n");

    return res;
}

static int setrfs_removexattr(const char *path, const char *name)
{
printf("#### NOT IMPLEMENTED! ####");
printf("setrfs_removexattr\n");

    return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations setrfs_oper = {
        .init        = setrfs_init,
        .getattr    = setrfs_getattr,
        .access        = setrfs_access,
        .readlink    = setrfs_readlink,
        .readdir    = setrfs_readdir,
        .mknod        = setrfs_mknod,
        .mkdir        = setrfs_mkdir,
        .symlink    = setrfs_symlink,
        .unlink        = setrfs_unlink,
        .rmdir        = setrfs_rmdir,
        .rename        = setrfs_rename,
        .link        = setrfs_link,
        .chmod        = setrfs_chmod,
        .chown        = setrfs_chown,
        .truncate    = setrfs_truncate,
#ifdef HAVE_UTIMENSAT
        .utimens	= setrfs_utimens,
#endif
        .open        = setrfs_open,
        .read        = setrfs_read,
        .write        = setrfs_write,
        .statfs        = setrfs_statfs,
        .release    = setrfs_release,
        .fsync        = setrfs_fsync,
#ifdef HAVE_POSIX_FALLOCATE
        .fallocate	= setrfs_fallocate,
#endif
#ifdef HAVE_SETXATTR
.setxattr	= setrfs_setxattr,
    .getxattr	= setrfs_getxattr,
    .listxattr	= setrfs_listxattr,
    .removexattr	= setrfs_removexattr,
#endif
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &setrfs_oper, NULL);
}
