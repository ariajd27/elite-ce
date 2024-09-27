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
#define BP_MAX_ENERGY		12

#define BP_SIDEWINDER		 0
#define BP_VIPER			 1
#define BP_MAMBA 			 2
#define BP_COBRA 			 3
#define BP_PYTHON			 4
#define BP_THARGOID 		 5
#define BP_CORIOLIS 		 6
#define BP_MISSILE 			 7
#define BP_ASTEROID 		 8
#define BP_CANISTER 		 9
#define BP_THARGON 			10
#define BP_ESCAPEPOD 		11
#define BP_NUM_SHIP_TYPES   12

extern unsigned char** bp_header_vectors[BP_NUM_SHIP_TYPES];

#endif
