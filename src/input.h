#ifndef input_include_file
#define input_include_file

extern bool yequ, graph, up, down, left, right, enter;
extern unsigned char prevYequ, prevGraph, prevUp, prevDown, prevLeft, prevRight, prevEnter;

void updateKeys();
void updatePrevKeys();

#endif
