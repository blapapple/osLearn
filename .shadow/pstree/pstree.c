#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// 进程节点结构体
struct ProcessNode {
    int pid;                   // 进程 ID
    int ppid;                  // 父进程 ID
    char name[256];            // 进程名称
    struct ProcessNode *next;  // 下一个兄弟进程
    struct ProcessNode *child; // 第一个子进程
};

// 创建进程节点
struct ProcessNode *create_process_node(int pid, int ppid, const char *name) {
    struct ProcessNode *node = (struct ProcessNode *)malloc(sizeof(struct ProcessNode));
    if (node != NULL) {
        node->pid = pid;
        node->ppid = ppid;
        strncpy(node->name, name, sizeof(node->name));
        node->next = NULL;
        node->child = NULL;
    }
    return node;
}

// 释放进程树
void release_process_tree(struct ProcessNode *root) {
    if (root != NULL) {
        release_process_tree(root->next);
        release_process_tree(root->child);
        free(root);
    }
}

// 添加子进程
void add_child_process(struct ProcessNode *parent, struct ProcessNode *child) {
    if (parent != NULL && child != NULL) {
        child->next = parent->child;
        parent->child = child;
    }
}

// 查找进程节点
struct ProcessNode *find_process_node(struct ProcessNode *root, int pid) {
    struct ProcessNode *node = root;
    while (node != NULL) {
        if (node->pid == pid) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// 打印进程树
void print_process_tree(struct ProcessNode *root, int level) {
    struct ProcessNode *node = root;
    while (node != NULL) {
        for (int i = 0; i < level; i++) {
            printf("  ");
        }
        printf("|- %d %s\n", node->pid, node->name);
        print_process_tree(node->child, level + 1);
        node = node->next;
    }
}
int main() {
  DIR *dir;
    struct dirent *entry;

    // 打开 /proc 目录
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // 创建根节点
    struct ProcessNode *root = create_process_node(0, 0, "root");

    // 遍历 /proc 目录中的文件
    while ((entry = readdir(dir)) != NULL) {
        // 如果是数字目录（进程目录），则添加到进程树中
        if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0) {
            int pid = atoi(entry->d_name);
            char name[256];
            int ppid = 0;
            // 读取进程名称和父进程ID
            char path[256];
            snprintf(path, sizeof(path), "/proc/%d/status", pid);
            FILE *file = fopen(path, "r");
            if (file != NULL) {
                while (fgets(name, sizeof(name), file)) {
                    if (strncmp(name, "Name:", 5) == 0) {
                        sscanf(name + 5, "%s", name);
                    } else if (strncmp(name, "PPid:", 5) == 0) {
                        sscanf(name + 5, "%d", &ppid);
                    }
                }
                fclose(file);
            }
            // 创建进程节点
            struct ProcessNode *node = create_process_node(pid, ppid, name);
            if (node != NULL) {
                // 添加进程到进程树中
                struct ProcessNode *parent = find_process_node(root, ppid);
                if (parent != NULL) {
                    add_child_process(parent, node);
                }
            }
        }
    }
    closedir(dir);

    // 打印进程树
    printf("Process Tree:\n");
    print_process_tree(root->child, 0);

    // 释放进程树
    release_process_tree(root->child);
  return 0;
}
