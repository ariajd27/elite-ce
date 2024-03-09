#ifndef input_include_file
#define input_include_file

extern bool yequ, graph, up, down, left, right, enter, math, vars, clear;
extern unsigned char prevYequ, prevGraph, prevUp, prevDown, prevLeft, prevRight, prevEnter, prevMath, prevVars, prevClear;

void updateKeys();
void updatePrevKeys();

char getChar();

#endif
