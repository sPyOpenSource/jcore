#include "vfs.h"
#include "fat32.h"
#include <stddef.h>

static struct vfs_mount mounts[VFS_MAX_FS];
static struct vfs_file files[VFS_MAX_FD];

void vfs_init(void) {
    for (int i = 0; i < VFS_MAX_FS; i++)
        mounts[i].used = 0;
    for (int i = 0; i < VFS_MAX_FD; i++)
        files[i].used = 0;
}

int vfs_mount(const char *path, struct vfs_ops *ops, void *device) {
    for (int i = 0; i < VFS_MAX_FS; i++) {
        if (!mounts[i].used) {
            mounts[i].used = 1;
            int j;
            for (j = 0; path[j] && j < VFS_NAME_LEN - 1; j++)
                mounts[i].path[j] = path[j];
            mounts[i].path[j] = '\0';
            mounts[i].ops = ops;
            mounts[i].fs_private = NULL;
            if (ops->mount(device, &mounts[i].fs_private) != 0) {
                mounts[i].used = 0;
                return -1;
            }
            return 0;
        }
    }
    return -2;
}

/* Find the mount point matching the path's prefix */
static struct vfs_mount *find_mount(const char *path) {
    struct vfs_mount *best = NULL;
    int best_len = 0;

    for (int i = 0; i < VFS_MAX_FS; i++) {
        if (!mounts[i].used) continue;

        int plen = 0;
        while (mounts[i].path[plen]) plen++;

        int match = 1;
        for (int j = 0; j < plen; j++) {
            if (mounts[i].path[j] != path[j]) {
                match = 0;
                break;
            }
        }

        if (match && plen >= best_len) {
            best = &mounts[i];
            best_len = plen;
        }
    }

    return best;
}

int vfs_open(const char *path, int flags) {
    (void)flags;
    struct vfs_mount *m = find_mount(path);
    if (!m) return -1;

    /* Find free FD */
    struct vfs_file *vf = NULL;
    int fd = -1;
    for (int i = 0; i < VFS_MAX_FD; i++) {
        if (!files[i].used) {
            vf = &files[i];
            fd = i;
            break;
        }
    }
    if (!vf) return -2;

    /* Strip mount prefix to get filesystem path */
    int plen = 0;
    while (m->path[plen]) plen++;
    const char *fs_path = path + plen;
    if (*fs_path == '/') fs_path++;
    if (*fs_path == '\0') fs_path = "";

    vf->used = 1;
    vf->mount = m;
    vf->pos = 0;

    if (m->ops->open(m->fs_private, fs_path, vf) != 0) {
        vf->used = 0;
        return -3;
    }

    return fd;
}

int vfs_read(int fd, void *buf, uint32_t size) {
    if (fd < 0 || fd >= VFS_MAX_FD) return -1;
    struct vfs_file *vf = &files[fd];
    if (!vf->used) return -2;

    int n = vf->mount->ops->read(vf, buf, size, vf->pos);
    if (n > 0) vf->pos += n;
    return n;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FD) return -1;
    struct vfs_file *vf = &files[fd];
    if (!vf->used) return -2;

    vf->mount->ops->close(vf);
    vf->used = 0;
    return 0;
}


