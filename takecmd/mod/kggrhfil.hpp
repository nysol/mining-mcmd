/* GRHFIL: convert graph format */
/* 2004 Takeaki Uno */
/* matdraw */

// #define FSTAR_INT unsigned int
// internal_params.l1 :  #edges written to the output file


#define GRHFIL_INS_ROWID 1024
#define GRHFIL_NORMALIZE 65536
#define GRHFIL_DISCRETIZE 131072

#define WEIGHT_DOUBLE

#include <math.h>
#include "stdlib2.hpp"
#include "fstar.hpp"
#include "itemset.hpp"


class KGGRHFIL{

  // PROBLEM _PP; //problemは使わない

  // problem項目
  ITEMSET _II;
  int _problem,_problem2;
  int _dir,_root;
  
  double _ratio;
  double _th,_th2;

  char *_weight_fname, *_table_fname;
  char *_output_fname;

  VEC_ID _rows;

  FSTAR _FS;
  FSTAR _FS2;
  
  
  char *_ERROR_MES ;

	void help(void);

	int read_param_iter (char *a, int *ff);
	void read_param (int argc, char *argv[]);
	void preLOAD(){

	  if ( _FS.get_fname() ){
	  	_FS.load();    
  		if (_ERROR_MES) EXIT;
  	}

	  if ( _FS2.get_fname() ){
		  _FS2.load();
  		if (_ERROR_MES) EXIT;
  	}
	}

	public:


	KGGRHFIL(){

		_problem  = 0;
		_problem2 = 0;
  	_dir  = 0;
  	_root = 0;
  	_ratio= 0;
		_th   = 0;
		_th2  = 0;
		_rows = 0;
		_weight_fname = NULL;
		_table_fname  = NULL;
		_output_fname = NULL;
		_ERROR_MES = NULL;
	
	}

	int run(int argc ,char* argv[]);
	static int mrun(int argc ,char* argv[]);

};


