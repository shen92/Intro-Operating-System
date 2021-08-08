#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <ctype.h>
#include "fs.h"

// [ boot block | super block | log | inode blocks | free bit map | data blocks]
int fd;                     //fs_image file descriptor
struct stat fs_stat;        //fs_image file information
void *ifp;                  //fs_image pointer
struct superblock *sbp;     //super block pointer
struct dinode *dip;         //disk inode pointer
char *bp;                   //bitmap pointer
int *db;                    //data_blocks array
int *db_bp;                 //data_blocks array from bitmap
int *db_i;                  //data_blocks array from inode

struct dinode *rdp;         //root directory pointer

//Open image file and get file system stat
void get_image(char argv[]) {
    fd = open(argv, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: image not found.\n");
        exit(1);
    }
    if (fstat(fd, &fs_stat) != 0) {
        fprintf(stderr, "ERROR: fstat failed.\n");
        exit(1);
    }
    ifp = mmap(NULL, fs_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ifp == MAP_FAILED) {
        fprintf(stderr, "ERROR: mmap failed.\n");
        exit(1);
    }
}

//Initialize global variables
void init_fields() {
    sbp = (struct superblock *) (ifp + BSIZE);
    dip = (struct dinode *) (ifp + BSIZE * sbp->inodestart);
    bp = (char *) (ifp + BSIZE * sbp->bmapstart);
    db = malloc(sizeof(int) * sbp->size);
    if (db == NULL) {
        fprintf(stderr, "ERROR: unable to allocate memory.\n");
        exit(1);
    }
    for (int i = 0; i < sbp->size; i++) {
        db[i] = 0;
    }
    db_bp = malloc(sizeof(int) * sbp->size);
    if (db_bp == NULL) {
        fprintf(stderr, "ERROR: unable to allocate memory.\n");
        exit(1);
    }
    for (int i = 0; i < sbp->size; i++) {
        db_bp[i] = 0;
    }
    db_i = malloc(sizeof(int) * sbp->size);
    if (db_i == NULL) {
        fprintf(stderr, "ERROR: unable to allocate memory.\n");
        exit(1);
    }
    for (int i = 0; i < sbp->size; i++) {
        db_i[i] = 0;
    }
    rdp = (struct dinode *) (ifp + BSIZE * sbp->inodestart) + 1;
}

void fs_check() {
    for (int i = 0; i < sbp->ninodes; i++, dip++) {
        //Case 1: Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV).
        if (dip->type < 0 || dip->type > 3) {
            fprintf(stderr, "ERROR: bad inode.\n");
            exit(1);
        } else if (dip->type != 0) {
            //Case 2: For in-use inodes, the size of the file is in a valid range given the number of valid datablocks.
            int db = 0;
            for (i = 0; i < NDIRECT; i++) {
                if (dip->addrs[i] != 0)db++;
            }
            int idb = 0;
            if (dip->addrs[NDIRECT] != 0) {
                uint *ib = (uint * )(ifp + dip->addrs[NDIRECT] * BSIZE);
                for (i = 0; i < NINDIRECT; i++) {
                    if (*ib != 0)idb++;
                    ib++;
                }
            }
            db += idb;
            if ((int) db * BSIZE < (int) dip->size ||
                (int) ((db - 1) * BSIZE) >= (int) dip->size) {
                fprintf(stderr, "ERROR: bad size in inode.\n");
                exit(1);
            }
        }
    }

    //Case 3: Root directory exists, and it is inode number 1.
    if (rdp->type != 1) {
        fprintf(stderr, "ERROR: root directory does not exist.\n");
        exit(1);
    }

    struct dirent *pwd = ifp + (BSIZE * rdp->addrs[0]);
    if (pwd->inum == 1 && (pwd + 1)->inum != 1) {
        fprintf(stderr, "ERROR: root directory does not exist.\n");
        exit(1);
    }

    dip = (struct dinode *) (ifp + BSIZE * sbp->inodestart);
    for (int i = 0; i < sbp->ninodes; i++, dip++) {
        //Case 4: The . entry in each directory refers to the correct inode.
        if (dip->type == 1) {
            struct dirent *pwd = (struct dirent *) (ifp + dip->addrs[0] * BSIZE);
            if (pwd->inum != i) {
                fprintf(stderr, "ERROR: current directory mismatch.\n");
                exit(1);
            }
        }
    }

    dip = (struct dinode *) (ifp + BSIZE * sbp->inodestart);
    for (int i = 0; i < sbp->ninodes; i++, dip++) {
        if (dip->type != 0) {
            for (int j = 0; j < NDIRECT; j++) {
                if (dip->addrs[j] != 0) {
                    db[dip->addrs[j]] = 1;
                }
            }
            if (dip->addrs[NDIRECT] != 0) {
                db[dip->addrs[NDIRECT]] = 1;
                uint *ib = (uint * )(ifp + dip->addrs[NDIRECT] * BSIZE);
                for (int j = 0; j < NINDIRECT; j++, ib++) {
                    if (*ib != 0)
                        db[*ib] = 1;
                }
            }
        }
    }

    bp = (char *) (ifp + BSIZE * sbp->bmapstart);
    for (int i = 0, k = 0; i < (sbp->nblocks / 8 + 1); i++, bp++) {
        for (int j = 0; j < 8; j++, k++) {
            db_bp[k] = (*bp >> (j)) & 1;
        }
    }

    bp = (char *) (ifp + BSIZE * sbp->bmapstart);
    for (int i = (sbp->bmapstart + 1); i < sbp->size; i++) {
        //Case 5: Each data block that is in use (pointed to by an allocated inode), is also marked in use in the bitmap.
        if (db[i] == 1 && db_bp[i] == 0) {
            fprintf(stderr, "ERROR: bitmap marks data free but data block used by inode.\n");
            exit(1);
        }
        //Case 6: For data blocks marked in-use in the bitmap, actually is in-use in an inode or indirect block somewhere.
        if (db[i] == 0 && db_bp[i] > 0) {
            fprintf(stderr, "ERROR: bitmap marks data block in use but not used.\n");
            exit(1);
        }
    }

    dip = (struct dinode *) (ifp + BSIZE * sbp->inodestart);
    int flag;
    for (int i = 0; i < sbp->ninodes; i++, dip++) {
        if (dip->type == 1) {
            flag = 0;
            for (int j = 0; j < NDIRECT; j++) {
                if (flag != 1) {
                    struct dirent *pwd = (struct dirent *) (ifp + dip->addrs[j] * BSIZE);
                    for (int k = 0; k < BSIZE / sizeof(struct dirent *); k++, pwd++) {
                        struct dinode *ip =
                                (struct dinode *) (ifp + BSIZE * sbp->inodestart) + pwd->inum;
                        //Case 7: For inode numbers referred to in a valid directory, actually marked in use in inode table.
                        if (pwd->inum != 0) {
                            if (ip->type == 0) {
                                fprintf(stderr, "ERROR: inode marked free but referred to in directory.\n");
                                exit(1);
                            }
                            db_i[pwd->inum] = 1;
                        } else {
                            flag = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    dip = (struct dinode *) (ifp + BSIZE * sbp->inodestart);
    for (int i = 0; i < sbp->ninodes; i++, dip++) {
        if (dip->type != 0) {
            //Case 8: For inodes marked used in inode table, must be referred to in at least one directory.
            if (db_i[i] != 1) {
                fprintf(stderr, "ERROR: inode marked in use but not found in a directory.\n");
                exit(1);
            }
        }
    }
}

void free_fields() {
    free(db);
    free(db_bp);
    free(db_i);
    munmap(ifp, fs_stat.st_size);
    close(fd);
}

int main(int argc, char *argv[]) {
    //Check argument
    if (argc < 2) {
        fprintf(stderr, "Usage: ./fscheck <file_system_image>\n");
        exit(1);
    }

    get_image(argv[1]);
    init_fields();
    fs_check();
    free_fields();

    return 0;
}

