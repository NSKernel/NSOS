#include <os.h>
#include <vfs.h>
#include <kmt.h>
#include <string.h>
#include <simple_lock.h>
#include <os/syslog.h>
 
void follow_dotdot(struct nameidata *nd);
int do_lookup(struct nameidata *nd, const char *name, int namelen);

struct dentry *path_lookup(const char *name, unsigned int flags, struct nameidata *nd) {
    const char *namestart;
    int namelen;
    
    if (*name == '/') { // lookup from root
         nd->dentry = current->fs.root;
         nd->mnt = current->fs.rootmnt;
    }
    else {
        nd->dentry = current->fs.pwd;
        nd->mnt = current->fs.pwdmnt;
    }
    
    while (*name == '/')
        name++;
            
    if (*name == '\0') { // the whole path is ////////...////, return the root dentry
        return nd->dentry;
    }
    
    for (;;) {
        namestart = name;
        namelen = 0;
        while (*name && (*name != '/')) {
            name++;
            namelen += 1;
        }
        
        if (*name) {
            while (*++name == '/');
            if (!*name && *(name - 1) == '/') // looking up a dir: final char of the path is '/'
                flags |= LOOKUP_DIRECTORY;
        }
        
        // . and .. 
        if (namestart[0] == '.') { 
            switch (namelen) {
              default:  // just a file name starts with .
                break;
              case 2:
                if (namestart[1] != '.') // just a file name starts with .
                  break;
                follow_dotdot(nd);
                /* fall through */
              case 1:
                continue;
            }
        }
        
        if (do_lookup(nd, namestart, namelen)) {
            // not found
            return NULL;
        }
        if (*name) {
            // we do need to make sure that nd->dentry is a dir
            if (nd->dentry->d_isdir)
                continue;
            else
                return NULL; // *found* a file but expected a dir
            continue;
        }
        
        if ((flags & LOOKUP_DIRECTORY) && !nd->dentry->d_isdir) {
            return NULL;
        }
        return nd->dentry;
    }
}

struct dentry *make_path_file(const char *name, struct nameidata *nd) { // the path needs to be a directory
    const char *namestart;
    char last_part = 0;
    int namelen;
    struct dentry *newdentry;
    struct inode *newinode;
    
    if (*name == '/') { // lookup from root
         nd->dentry = current->fs.root;
         if (nd->dentry != root_dentry) {
         }
         nd->mnt = current->fs.rootmnt;
    }
    else {
        nd->dentry = current->fs.pwd;
        nd->mnt = current->fs.pwdmnt;
    }
    
    while (*name == '/')
        name++;
            
    if (*name == '\0') { // the whole path is ////////...////, still a path
        return NULL;
    }
    
    for (;;) {
        namestart = name;
        namelen = 0;
        while (*name && (*name != '/')) {
            name++;
            namelen += 1;
        }
        
        if (*name) {
            while (*++name == '/');
            if (!*name && *(name - 1) == '/') // making a path
                return NULL;
        }
        if (!*name) {
            last_part = 1;
        }
        
        // . and .. 
        if (namestart[0] == '.') { 
            switch (namelen) {
              default:  // just a file name starts with .
                break;
              case 2:
                if (namestart[1] != '.') // just a file name starts with .
                  break;
                follow_dotdot(nd);
                /* fall through */
              case 1:
                if (last_part)
                    return NULL; // a path
                continue;
            }
        }
        
        if (do_lookup(nd, namestart, namelen)) {
            if(check_acl(current->uid, current->gid, nd->dentry->d_inode, R_OK | W_OK)) {
                return NULL;
            }
            
            newinode = nd->mnt->mnt_fs->sop->alloc_inode(nd->mnt->mnt_fs);
            newinode->i_uid = current->uid;
            newinode->i_gid = current->gid;
            newinode->i_acl = ACL_DEFAULT;
            newinode->i_op = nd->mnt->mnt_fs->iop; 
            
            newdentry = pmm->alloc(sizeof(struct dentry));
            newdentry->d_iname = pmm->alloc(sizeof(char) * (namelen + 1));
            strncpy(newdentry->d_iname, namestart, namelen + 1);
            newdentry->d_iname[namelen] = '\0';
            
            newdentry->d_inode = newinode;
            newdentry->d_parent = nd->dentry;
            newdentry->d_child = nd->dentry->d_child;
            newdentry->d_subdirs = NULL;
            newdentry->d_mounted = 0;
            newdentry->d_op = nd->mnt->mnt_fs->dop;
            
            if (last_part) { // make a file
                if (newinode->i_op->create(newinode, newdentry)) {
                    syslog("VFS", "failed to create a file inode in create");
                    nd->mnt->mnt_fs->sop->destroy_inode(newinode);
                    pmm->free(newdentry);
                    return NULL;
                }
            }
            else { // make a directory
                if (newinode->i_op->mkdir(newinode, newdentry)) {
                    syslog("VFS", "failed to create a directory inode in mkdir");
                    nd->mnt->mnt_fs->sop->destroy_inode(newinode);
                    pmm->free(newdentry);
                    return NULL;
                }
            }
            
            newdentry->d_child = nd->dentry->d_subdirs;
            nd->dentry->d_subdirs = newdentry;
            nd->dentry = newdentry;
        }
        // nd->dentry = do_lookup: if the previous do_lookup succeed, the nd->detnry should have already been updated.
        
        if (*name) {
            // but we do need to make sure that nd->dentry is a dir
            if (nd->dentry->d_isdir)
                continue;
            else
                return NULL; // *found* a file but expected a dir
        }
        
        // no more chars in name
        if (nd->dentry->d_isdir) // *found* a dir: make will ensure it makes a file
            return NULL;
        return nd->dentry;
    }
}

void follow_dotdot(struct nameidata *nd) {
    if (nd->dentry == root_dentry) {
        return; // nothing will happen
    }

    if (nd->dentry->d_parent == nd->dentry) { // root of a mounted fs, needs to go back to the upper fs
        nd->dentry = nd->mnt->mnt_mountpoint;
        nd->mnt = nd->mnt->mnt_parent;
    }
    nd->dentry = nd->dentry->d_parent;
}

int do_lookup(struct nameidata *nd, const char *name, int namelen) {
    struct vfsmount *mntit = mnt_head;
    struct dentry *denit;
    
    
    if (nd->dentry->d_mounted) { // if mounted
        while (mntit != NULL && mntit->mnt_mountpoint != nd->dentry) // pick the last mounted fs
            mntit = mntit->mnt_next;
        if (mntit == NULL) {
            syslog("VFS", "path lookup failed in follow_dotdot. mount issue.");
            panic("VFS found a mount issue");
        }
        
        nd->dentry = mntit->mnt_root;
        nd->mnt = mntit;
    }
    
    denit = nd->dentry->d_subdirs;
    
    while (denit != NULL) {
        if (strlen(denit->d_iname) != namelen || strncmp(denit->d_iname, name, namelen)) {
            denit = denit->d_child;
            continue;
        }
        else {
            if (denit->d_op->d_revalidate != NULL || !denit->d_op->d_revalidate(denit)) { // revalidate if the dentry is still valid. if no revalidate function avaliable, regard as always valid
                nd->dentry = denit;
                return 0;
            }
            else { // the dentry is found but is not valid. drop it and use slower lookup
                d_drop(denit);
                break;
            }
        }
    }
    
    if (nd->dentry->d_inode->i_op->lookup != NULL) { // still got a chance to lookup for a subdir that has not been read yet
        denit = (struct dentry *)(pmm->alloc(sizeof(struct dentry)));
        denit->d_iname = (char *)(pmm->alloc(sizeof(char) * (namelen + 1)));
        strncpy(denit->d_iname, name, namelen + 1);
        denit->d_iname[namelen] = '\0';
        denit->d_inode = NULL;
        denit->d_parent = nd->dentry;
        denit->d_child = nd->dentry->d_child;
        denit->d_subdirs = NULL;
        denit->d_mounted = 0;
        denit->d_op = nd->mnt->mnt_fs->dop;
        
        if (nd->dentry->d_inode->i_op->lookup(nd->dentry->d_inode, denit) != NULL) {
            denit->d_child = nd->dentry->d_subdirs;
            nd->dentry->d_subdirs = denit;
            nd->dentry = denit;
            return 0;
        }
    }
    return 1;
}
