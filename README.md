# TecnicoFS

#### Usage ####
```/tecnicofs input.txt output.txt numberOfThreads```

#### Input file example ####
```c /a d - Creates a directory /a
c /a/b f - Creates a file /a/b
l /a/b - Lookup for file /a/b
c /a/b/c d - Creates a directory /a/b/c -> Will fail (impossible)
c /a/d - Creates a directory /a/d 
m /a/b/c /a/d - Moves /a/b/c to /a/d 
```
