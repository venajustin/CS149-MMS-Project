# CS149 Memory Management System Project
As our largest project in my Operating Systems class we were assigned the task of creating a memory management system. This emulates something similar to malloc/free/memcopy in C.

Memory is allocated from a shared memory region that can be accessed by other programs including the header file.

There is a known bug with the copy operation. Originally it allowed for copying from any source (external or internal buffer) into any other source. Due to clarification on project requirements this was changed and introduced a bug where errors are thrown on what should be valid operations.
