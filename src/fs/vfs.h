#ifndef VFS_H
#define VFS_H

#include <stdint.h>

#define VFS_MAX_FD    16
#define VFS_MAX_FS    4
#define VFS_NAME_LEN  64
#define VFS_PRIV_SIZE 32 /* max bytes for FS-private file handle */

struct vfs_file;

/* Filesystem operations table */
struct vfs_ops {
    int  (*mount)(void *device, void **fs_private);
    int  (*open)(void *fs_private, const char *path, struct vfs_file *vf);
    int  (*read)(struct vfs_file *vf, void *buf, uint32_t size, uint32_t offset);
    void (*close)(struct vfs_file *vf);
    int  priv_size; /* size of FS-private file handle */
};

/* Mount point */
struct vfs_mount {
    char path[VFS_NAME_LEN];
    struct vfs_ops *ops;
    void *fs_private;
    int used;
};

/* VFS file descriptor (per open file) */
struct vfs_file {
    struct vfs_mount *mount;
    uint32_t pos;
    int used;
    uint8_t priv[VFS_PRIV_SIZE]; /* FS-private file handle storage */
};

/* API */
void vfs_init(void);
int vfs_mount(const char *path, struct vfs_ops *ops, void *device);
int vfs_open(const char *path, int flags);
int vfs_read(int fd, void *buf, uint32_t size);
int vfs_close(int fd);

#endif /* VFS_H */
