#ifndef shipdata_include_file
#define shipdata_include_file

#define BP_GENERAL			 0
#define BP_VERTICES			 1
#define BP_EDGES			 2
#define BP_FACES 			 3

#define BP_GENERAL_LENGTH	16

#define BP_VERT_L_IND		 6
#define BP_L_VERT 			 6

#define BP_EDGE_Q_IND		 7
#define BP_L_EDGE			 4

#define BP_FACE_L_IND		10
#define BP_L_FACE 			 4

#define BP_VISDIST_IND		11
#define BP_EXPLCT_IND		 5
#define BP_MAX_SPEED		13

#define BP_SIDEWINDER		 0
#define BP_VIPER			 1
#define BP_MAMBA 			 2
#define BP_COBRA 			 3
#define BP_THARGOID 		 4
#define BP_CORIOLIS 		 5
#define BP_MISSILE 			 6
#define BP_ASTEROID 		 7
#define BP_CANISTER 		 8
#define BP_THARGON 			 9
#define BP_ESCAPEPOD 		10

extern unsigned char** bp_header_vectors[11];

#endif
