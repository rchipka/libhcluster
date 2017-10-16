#ifndef HCLUSTER_H
#define HCLUSTER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HC_DEBUG

typedef struct hc_link hc_link_t;
typedef struct hc_cluster hc_cluster_t;
typedef struct hc_level hc_level_t;
typedef struct hc_link_args hc_link_args_t;
typedef float (*hc_distance_function_t)(void *a, void *b, void *extra);
typedef void* (*hc_value_function_t)(void *values, int index);

struct hc_link {
  int index;
  int count;
  int *clusters;
  float distance;
  hc_cluster_t *source;
  hc_cluster_t *target;
};

struct hc_link_opts_t {
  hc_cluster_t *source;
  hc_cluster_t *target;
  void *args;
};

struct hc_cluster {
  int index;
  int offset;
  int length;
  hc_link_t *link;
  int removed;
  hc_link_t *minimum;
  hc_cluster_t *prev;
  hc_cluster_t *next;
  hc_cluster_t *cluster;
  void *value;
};

struct hc_level {
  float linkage;
  int source;
  int target;
  hc_link_t *link;
  hc_level_t *next;
  int *clusters;
};

hc_cluster_t* hc_new(void *values, int length, hc_value_function_t value);
hc_link_t*    hc_link(hc_cluster_t *clusters, hc_distance_function_t distance, void *extra);
hc_level_t*   hc_cluster(hc_cluster_t *clusters, hc_link_t *links, int length);

hc_link_t    *hc_next(hc_cluster_t *clusters, hc_link_t *links, hc_link_t *lastLink);
hc_link_t    *hc_merge_link(hc_link_t *source, hc_link_t *target);
void          hc_merge_cluster(hc_cluster_t *source, hc_cluster_t *target);
void          hc_unlink(hc_cluster_t *cluster);


#endif
