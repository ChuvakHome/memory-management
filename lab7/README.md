## Memory allocators comparison

|           Pool           |       Time         | Overhead |
|:-------------------------|:-------------------|:---------|
| Standard                 |  6391728 &micro;s  |   50.0%  |
| Mutex global allocator   | 30753313 &micro;s  |   -0.1%  |
| Lock-free allocator      | 63983720 &micro;s  |   -0.3%  |
| Thread-local allocator   |   401612 &micro;s  |  -29.9%  |
