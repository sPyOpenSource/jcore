#include "vfs.h"
#include "fat32.h"
#include <stddef.h>
#include <stdint.h>

static struct fat32_fs fat32_fs_instance;

static int fat32_vfs_mount(void *device, void **fs_private) {
    *fs_private = &fat32_fs_instance;
    return fat32_mount((struct mmc_block_dev *)device, &fat32_fs_instance);
}

static int fat32_vfs_open(void *fs_private, const char *path, struct vfs_file *vf) {
    (void)fs_private;
    struct fat32_file *fh = (struct fat32_file *)vf->priv;
    return fat32_open_root(&fat32_fs_instance, path, fh);
}

static int fat32_vfs_read(struct vfs_file *vf, void *buf, uint32_t size, uint32_t offset) {
    struct fat32_file *fh = (struct fat32_file *)vf->priv;
    return fat32_read(fh, buf, size, offset);
}

static void fat32_vfs_close(struct vfs_file *vf) {
    (void)vf;
}

struct vfs_ops fat32_vfs_ops = {
    .mount = fat32_vfs_mount,
    .open  = fat32_vfs_open,
    .read  = fat32_vfs_read,
    .close = fat32_vfs_close,
    .priv_size = sizeof(struct fat32_file),
};
