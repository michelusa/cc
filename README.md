# cc
reclamation: safe reclamation of dynamically allocated memory. one writer, multiple readers with lockfree access to 
allocated memory. 
A garbage collector (with mutex - synchronized old version list) do the freeing, ran on a reader thread.
