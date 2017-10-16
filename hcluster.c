#include "hcluster.h"

void hc_init_cluster(hc_cluster_t *cluster) {
  cluster->index    = 0;
  cluster->offset   = 0;
  cluster->length   = 0;
  cluster->removed  = 0;
  cluster->link     = NULL;
  cluster->prev     = NULL;
  cluster->next     = NULL;
  cluster->minimum  = NULL;
  cluster->cluster  = NULL;
  cluster->value    = NULL;
}

hc_cluster_t* hc_new(void* values, int length, hc_value_function_t getValue) {
  hc_cluster_t *clusters = (hc_cluster_t*) malloc((length + 1) * sizeof(hc_cluster_t));

  hc_cluster_t *curr;
  hc_cluster_t *prev;

  #ifdef DEBUG
  fprintf(stderr, "hc_new: length=%i\n", length);
  #endif

  hc_init_cluster(&clusters[0]);

  for (int i = 0; i < length; i++) {
    prev = &clusters[i];
    curr = &clusters[i + 1];

    hc_init_cluster(curr);

    curr->index     = i;
    curr->prev      = prev;
    curr->value     = getValue(values, i);
    prev->next      = curr;
  }

  return clusters;
}

hc_link_t* hc_link(hc_cluster_t *clusters, hc_distance_function_t distance, void *extra) {
  int length = 0;
  int offset = 0;
  int total  = 0;

  hc_cluster_t* cluster = clusters;

  // printf("cluster %p %p\n", cluster, cluster->next);
  for (int i = 0; (cluster = cluster->next); i++, length++) {
    total += i;
  }

  #ifdef DEBUG
  // fprintf(stderr, "hc_link: allocating %i links\n", total);
  #endif

  hc_link_t *links = (hc_link_t*) malloc(total * sizeof(hc_link_t));

  hc_link_t *link;

  hc_cluster_t *curr;
  hc_cluster_t *prev;

  for (int i = 0; i < length; i++) {
    prev = &clusters[i];
    curr = &clusters[i + 1];

    curr->offset    = offset;

    #ifdef DEBUG
    // fprintf(stderr, "hc_link: linking i=%i curr->offset=%i\n", i, curr->offset);
    #endif

    while (prev->prev != NULL) {
      #ifdef DEBUG
      // fprintf(stderr, "hc_link: linking %p => %p\n", prev->value, curr->value);
      // fprintf(stderr, "hc_link: linking prev->index=%i\n", prev->index);
      #endif

      link = &links[curr->offset + prev->index];

      link->source = prev;
      link->target = curr;
      link->count = 1;
      link->distance = distance(prev->value, curr->value, extra);
      curr->minimum = NULL;

      #ifdef DEBUG
      // fprintf(stderr, "\t%i => %i   -   %f\n", prev->index, curr->index, link->distance);
      #endif

      if (curr->minimum == NULL ||
          link->distance < curr->minimum->distance) {
        curr->minimum = link;
      }

      prev = prev->prev;

      offset++;
    }
  }

  return links;
}

float hc_average(hc_link_t *source, hc_link_t *target) {
  return (
    (source->distance * source->count) +
    (target->distance * target->count)) /
    (target->count += source->count);
}

hc_link_t *min(hc_link_t *a, hc_link_t* b) {
  if (a == NULL) {
    return b;
  }

  if (b == NULL) {
    return a;
  }

  if (a->distance < b->distance) {
    return a;
  }

  return b;
}

hc_link_t* hc_merge_link(hc_link_t *source, hc_link_t *target) {
  target->distance = hc_average(source, target);
  return target;
}

hc_link_t* hc_update(hc_cluster_t *cluster, hc_link_t *links) {
  hc_link_t *min = NULL, *link;
  int offset = cluster->offset;

  while ((cluster = cluster->prev) != NULL) {
    link = &links[offset + cluster->index];

    if (link->source->removed == 0 &&
        (min == NULL ||
         link->distance < min->distance)) {
      min = link;
    }
  }

  return min;
}

hc_link_t *hc_next(hc_cluster_t *cluster, hc_link_t *links, hc_link_t *lastLink) {
  int index = 0;
  hc_link_t *link, *minLink = NULL;

  while (cluster != NULL) {
    if (cluster->minimum != NULL && cluster->minimum->source->removed == 1) {
      cluster->minimum = hc_update(cluster, links);
    }

    if (lastLink != NULL) {
      hc_cluster_t *source = lastLink->source;
      hc_cluster_t *target = lastLink->target;

      // lastLink->clusters[index++] = cluster->index;
      // lastLink->clusters[index++] = cluster->length;

      if (cluster->index > target->index) {
        cluster->minimum = min(
          hc_merge_link(
            &links[cluster->offset + source->index],
            &links[cluster->offset + target->index]
          ), cluster->minimum);
      } else if (cluster->index < target->index) {
        link = &links[source->offset + cluster->index];

        if (cluster->index > source->index) {
          link = &links[cluster->offset + source->index];
        }

        target->minimum = min(
          hc_merge_link(
            link,
            &links[target->offset + cluster->index]
          ), target->minimum);
      }
    }

    if (cluster->minimum != NULL &&
        (minLink == NULL ||
         cluster->minimum->distance < minLink->distance)) {
      minLink = cluster->minimum;
    }

    cluster = cluster->next;
  }

  return minLink;
}

void hc_unlink(hc_cluster_t *cluster) {
  cluster->removed = 1;
  cluster->next->prev = cluster->prev;

  if (cluster->prev != NULL) {
    cluster->prev->next = cluster->next;
  }
}

void hc_merge_cluster(hc_cluster_t *source, hc_cluster_t *target) {
  target->length += source->length;

  while (target->cluster != NULL) {
    target = target->cluster;
  }

  target->cluster = source;

  hc_unlink(source);
}

hc_level_t* hc_cluster(hc_cluster_t *cluster, hc_link_t *links, int length) {
  hc_cluster_t  *target, *source;
  hc_link_t    *link = NULL;

  hc_level_t *levels = (hc_level_t*) malloc(length * sizeof(hc_level_t));
  hc_level_t *level, *prev = NULL;
  int offset = 0;
  target = NULL;
  source = NULL;

  while ((link = hc_next(cluster, links, link)) != NULL) {
    target = link->target;
    source = link->source;

    link->index = offset;
    level = &levels[offset++];

    printf("malloc: %i\n", (length - offset));
    // link->clusters = (int*) malloc((length - offset) * sizeof(int) * 2);

    if (prev != NULL) {
      prev->next = level;
    }

    level->next     = NULL;
    level->linkage  = link->distance;
    level->source   = source->index;
    level->target   = target->index;
    // level->clusters = link->clusters;

    if (source->prev == NULL) {
      cluster = source->next;
    }

    source->link = link;

    hc_merge_cluster(source, target);

    prev = level;
  }

  return levels;
}
