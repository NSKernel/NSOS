#ifndef _PATH_H_
#define _PATH_H_

#include <kernel.h>
#include <vfs.h>

struct dentry *path_lookup(const char *name, unsigned int flags, struct nameidata *nd);
struct dentry *make_path_file(const char *name, struct nameidata *nd);

#endif
