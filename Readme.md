# LibHcluster

Simple C implementation of hierarchical agglomerative clustering.

## Usage

```c
float get_distance(some_t* a, some_t* b, some_extra* opts) {
  return abs(a - b);
}

void* get_value(void* values, int index) {
  return &array[index];
}

int total = 100;

some_t* values[total] = { ... };

hc_cluster_t* clusters = hc_new(values, total, get_value);
hc_link_t*    links    = hc_link(clusters, get_distance, &extra);
hc_level_t*   levels   = hc_cluster(clusters, links, total);
```

## Functions

### hc_new(void\* values, int length, hc_value_function_t value)

Create a new `hc_cluster_t*` array of clusters.

### hc_link(hc_cluster_t\* clusters, hc_distance_function_t distance, void\* extra)

Create a new `hc_link_t*`  array of links between each input, measured using
the provided `hc_distance_function_t` distance function.

### hc_cluster(hc_cluster_t\* clusters, hc_link_t\* links, int length)

Create a new `hc_level_t*` array of `length - 1` levels for each merged cluster.

### hc_free()

## Types

### (struct) hc_cluster_t
### (struct) hc_link_t
### (struct) hc_level_t

### (void\*) hc_value_function_t\* callback(void\* values, int index)

Return a void* to a value at the given `index`.
This value is used later for the distance callback.

### (void\*) hc_distance_function_t\* callback(void\* a, void\* b, void\* extra)

Return a float representing the distance between `a` and `b`.