#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_PROCESSES 4096

typedef struct PProcessNode
{
  int pid;
  int ppid;
  char name[256];
  struct PProcessNode *child;
  struct PProcessNode *brother;
} ProcessNode;
bool show_pids = false;
bool numeric_sort = false;

ProcessNode *create_process_node(int pid, int ppid, const char *name)
{
  ProcessNode *node = (ProcessNode *)malloc(sizeof(ProcessNode));
  if (node != NULL)
  {
    node->pid = pid;
    node->ppid = ppid;
    strcpy(node->name, name);
    node->child = NULL;
    node->brother = NULL;
  }
  return node;
}

ProcessNode *find_process_node(ProcessNode *root, int pid)
{
  if (root == NULL)
    return NULL;
  if (root->pid == pid)
    return root;
  ProcessNode *result = find_process_node(root->child, pid);
  if (result == NULL)
  {
    result = find_process_node(root->brother, pid);
  }
  return result;
}

void add_child_process(ProcessNode *parent, ProcessNode *child)
{
  if (child != NULL && parent != NULL)
  {
    child->brother = parent->child;
    parent->child = child;
  }
}

void print_process_tree(ProcessNode *root, int depth)
{
  if (root == NULL)
    return;
  // 打印当前进程
  for (int i = 0; i < depth; i++)
  {
    printf(" ");
  }
  if (show_pids)
  {
    printf("(%d)", root->pid);
  }
  printf("%s/n", root->name);
  print_process_tree(root->child, depth + 1);
  print_process_tree(root->brother, depth);
}

int compare_process(const void *a, const void *b)
{
  const ProcessNode *pa = *(const ProcessNode **)a;
  const ProcessNode *pb = *(const ProcessNode **)b;
  return pa->pid - pb->pid;
}

void release_process_tree(ProcessNode *root)
{
  if (root == NULL)
    return;
  release_process_tree(root->child);
  release_process_tree(root->brother);
  free(root);
}

ProcessNode *read_process()
{
  ProcessNode *root = NULL;
  ProcessNode *processes[MAX_PROCESSES];
  int count = 0;
  DIR *dir = opendir("/proc");
  if (dir == NULL)
  {
    perror("opendir");
    return NULL;
  }
  struct dirent *entry;                  // 目录项结构体
  while ((entry = readdir(dir)) != NULL) // 遍历目录
  {
    if (entry->d_type == DT_DIR)
    {
      int pid = atoi(entry->d_name);
      if (pid > 0)
      {
        char path[256]; // 存储进程状态文件路径
        sprintf(path, "/proc/%d/status", pid);
        FILE *file = fopen(path, "r");
        if (file == NULL)
        {
          perror("fopen");
          continue;
        }
        char line[256];
        int ppid = 0;
        char name[256];
        while (fgets(line, sizeof(line), file) != NULL)
        {
          if (sscanf(line, "Name:%s", name) == 1)
          {
          }
          else if (sscanf(line, "PPid:%d", &ppid))
        }
        fclose(file);
        ProcessNode *node = create_process_node(pid, ppid, name);
        if (node == NULL)
        {
          fprintf(stderr, "create_process_node failed\n");
          continue;
        }
        if (pid == 1)
        {
          root = node;
        }
        processes[count++] = node;
      }
    }
  }
  closedir(dir);
  if (numeric_sort) // 如果按照id排序
  {
    qsort(processes, count, sizeof(ProcessNode *), compare_process);
  }
  for (int i = 0; i < count; i++)
  {
    ProcessNode *node = processes[i];
    if (node->pid != 1)
    {
      ProcessNode *parent = find_process_node(root, node->pid);
      if (parent != NULL)
      {
        add_child_process(parent, node);
      }
    }
  }
  return root;
}

int main(int argc, char *argv[])
{
  int opt;
  while ((opt = getopt(argc, argv, "pnhV")) != -1)
  {
    switch (opt)
    {
    case 'p':
      show_pids = true;
      break;
    case 'n':
      numeric_sort = true;
      break;
    case 'V':
      printf("my pstree 1.0/n");
      return 0;
    case 'h':
    default:
      fprintf(stderr, "Usage: %s [-pnhV]\n", argv[0]);
      fprintf(stderr, "  -p, --show-pids\t\tPrint process PIDs\n");
      fprintf(stderr, "  -n, --numeric-sort\t\tSort by process ID\n");
      fprintf(stderr, "  -V, --version\t\t\tPrint version information\n");
      return 1;
    }
  }
  ProcessNode *root = read_process();
  if (root == NULL)
  {
    fprintf(stderr, "read_processes failed\n") return 1;
  }
  print_process_tree(root, 0);
  release_process_tree(root);
  return 0;
}
