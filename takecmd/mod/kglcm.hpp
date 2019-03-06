/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno,   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about LCM for the users.
   For the commercial use, please make a contact to Takeaki Uno. */

#pragma once


#define WEIGHT_DOUBLE
#define TRSACT_DEFAULT_WEIGHT 1

// use the complement graph of the constraint graph
#define LCM_UNCONST 16777216   

// an item will be dealt as "equisupp" when "positive"-frequency 
// is equal to the positive-frequency of the current itemset 
#define LCM_POSI_EQUISUPP 33554432   

#define ERROR_RET 

#include"trsact.hpp"
#include"sgraph.hpp"
#include"problem.hpp"


class KGLCM{

	ITEMSET _II;
	TRSACT _TT;
	SGRAPH _SG;

	int _problem;

	double _th;
	char* _outperm_fname;
	char* _output_fname;
	char* _t_pfname;

	//_II
	int _lb,_ub;
	int _iFlag; //,_iFlag2 LAMP用？
	double _prob_lb,_prob_ub;
	int _target;
	double _ratio_lb,_ratio_ub;
	double _nega_lb,_nega_ub;
	double _posi_lb,_posi_ub;
	double _rposi_ub,_rposi_lb;
	int _max_solutions;
	char _separator;
	int _digits;
	WEIGHT _frq_lb,_frq_ub;
	LONG _topk_k;

	//_TT
	int _tFlag,_tFlag2; //,_iFlag2 LAMP用？
	char *_fname;
	char *_pfname;
	char *_wfname;
	double _w_lb;


	// SG
	char *_sgfname;
	int _sgFlag;


	WEIGHT * _occ_w,*_occ_w2,*_occ_pw,*_occ_pw2;

  QUEUE_INT *_itemary;
	QUEUE _itemcand;
	QUEUE_INT _clms;
	QUEUE _oo;
	char* _ERROR_MES;

	void help();


	/* allocate arrays and structures */
	void preALOCC (){

		QUEUE_ID siz = _TT.get_clms();
		PERM *perm   = _TT.get_perm();

//		int f =
//			PROBLEM_ITEMCAND 
//			+ ( _sgfname ? PROBLEM_ITEMARY:0) 
//			+ ( (_tFlag2 & TRSACT_NEGATIVE)? PROBLEM_OCC_PW: PROBLEM_OCC_W ) 
//			+ ( (_problem&PROBLEM_FREQSET )? 0 : PROBLEM_OCC_W2 );

	  PERM *p=NULL;
  	int j;

  	// if ( f&(PROBLEM_OCC_W+PROBLEM_OCC_PW) ) calloc2 (_occ_w, siz+2, goto ERR);
		calloc2 (_occ_w, siz+2, goto ERR);

	  // if ( f&PROBLEM_OCC_PW ){ calloc2 (_occ_pw, siz+2, goto ERR); }
	  // else{ _occ_pw = _occ_w;}
		if ( _tFlag2 & TRSACT_NEGATIVE){
			calloc2 (_occ_pw, siz+2, goto ERR);
		}else{
			_occ_pw = _occ_w;
		}

	  //if ( f&PROBLEM_OCC_W2 ){
  	//  calloc2 (_occ_w2, siz+2, goto ERR);
    //	if ( f&PROBLEM_OCC_PW ) calloc2 (_occ_pw2, siz+2, goto ERR);
		//	else  _occ_pw2 = _occ_w2;
		//}
		//else {
		//	_occ_w2 = _occ_w;
		//	_occ_pw2 = _occ_pw; 
		//}
		if( _problem&PROBLEM_FREQSET){
			_occ_w2 = _occ_w;
			_occ_pw2 = _occ_pw; 
		}
		else{
  	  calloc2 (_occ_w2, siz+2, goto ERR);
    	if ( _tFlag2 & TRSACT_NEGATIVE ){
    		 calloc2 (_occ_pw2, siz+2, goto ERR);
    	}
			else{
				_occ_pw2 = _occ_w2;
			}
		}

		// if ( f&PROBLEM_ITEMARY ) calloc2(_itemary, siz+2, goto ERR);
	  // if ( f&PROBLEM_ITEMCAND ) _itemcand.alloc ( siz+2);
	  if(_sgfname) { calloc2(_itemary, siz+2, goto ERR); }
	  _itemcand.alloc ( siz+2);

	   // set outperm
	  if ( _outperm_fname ){
  	  j = FILE2::ARY_Load (p,  _outperm_fname, 1);
    	if ( perm ){
				FLOOP (j, 0, siz) perm[j] = p[perm[j]];
				free2 (p);
	    }
  	  else perm = p;
  	}

		_II.alloc(_output_fname, perm, siz, 0);

 	 if ( _target < siz && _II.exist_perm() ){
  	  FLOOP (j, 0, _II.get_item_max()){ 
    		if ( _II.get_target() == _II.get_perm(j) ){ _II.set_target(j); break; } 
    	}
  	}
	  return;
	  ERR:;

	  EXIT;
	}

	void _init (){


	  PERM *sperm = NULL, *tmp=NULL;
  	QUEUE_INT i;

	  //なにこれ..
	  _clms = ((_problem&PROBLEM_FREQSET)&&(_iFlag&ITEMSET_RULE)==0);

		preALOCC();

		// threshold for database reduction
		_th = (_iFlag&ITEMSET_RULE)? 
  			((_iFlag & ITEMSET_RULE_INFRQ)? -WEIGHTHUGE : _frq_lb * _ratio_lb ): _frq_lb;
  			 

		if ( _tFlag2 & TRSACT_SHRINK ){
  		_oo = _TT.dup_OQ(_TT.get_clms()); // preserve occ
  	}
  	else {
  		_oo.alloc(_TT.get_t()); 
			for(size_t i=0 ; i< _TT.get_t(); i++){ _oo.set_v(i,i); }
  	 	_oo.set_t( _TT.get_t()); 
  	}

		_TT.set_perm(NULL);
	  _TT.set_OQ_t(_TT.get_clms(),0);

		print_mesf (&_TT, "separated at %d\n", _TT.get_sep());

		if ( !(_TT.exist_sc()) ) _TT.calloc_sc( _TT.get_clms()+2);
		
		_II.set_itemflag(_TT.get_sc());

		_II.set_frq( _TT.get_total_w_org()); 
  	_II.set_pfrq( _TT.get_total_pw_org());


  if ( _sgfname ){
    if ( _SG.edge_t() < _TT.get_clms() )
        print_mesf (&_TT, "#nodes in constraint graph is smaller than #items\n");

    if ( _TT.exist_perm() ){
    
      malloc2 (sperm, _SG.edge_t(), EXIT);
      //ARY_INIT_PERM (sperm, _SG._edge.get_t());
			for(size_t i=0 ; i< _SG.edge_t(); i++){ sperm[i]=i; }

      FLOOP (i, 0, MIN(_TT.get_t(), _SG.edge_t())) sperm[i] = _TT.get_perm(i);

      //ARY_INV_PERM (tmp, sperm, _SG._edge.get_t(), {free(sperm);EXIT;});
			malloc2(tmp,_SG.edge_t(),{free(sperm);EXIT;});
			for(size_t st=0; st<_SG.edge_t() ;st++){ tmp[st]=-1; }
			for(int i=0;i<_SG.edge_t();i++){
				if(sperm[i]>=0 && sperm[i]<_SG.edge_t()){ tmp[sperm[i]]=i; }
			}
      _SG.replace_index (sperm, tmp);
      mfree (tmp, sperm);
      _SG.set_perm( NULL);
    }
    
    _SG.edge_union_flag(LOAD_INCSORT +LOAD_RM_DUP);
  	_SG.edge_sort();
  }

  _II.set_total_weight(_TT.get_total_w());
  if ( (_II.get_flag2()&ITEMSET_LAMP) && _II.get_topk_base() == 0) _II.set_topk_base( _TT.get_rows_org());

}	

	
	void read_param (int argc, char *argv[]);

	int setArgs(int argc, char *argv[]);


	void add_item ( QUEUE *Q, QUEUE_INT item);//未使用
	void add_item ( QUEUE_INT item);

	void del_item (QUEUE *Q);



	void reduce_occ_by_posi_equisupp ( QUEUE *occ, QUEUE_INT item, QUEUE_INT full);

  QUEUE_INT maximality_check (QUEUE *occ, QUEUE_INT item, QUEUE_INT *fmax, QUEUE_INT *cnt);

	void LCMCORE (int item, QUEUE *occ, WEIGHT frq, WEIGHT pfrq);


	public:

	KGLCM():
		_problem(0),_th(0),_occ_w(NULL),_occ_w2(NULL),_occ_pw(NULL),_occ_pw2(NULL),
		_clms(0),_outperm_fname(NULL),_output_fname(NULL),_itemary(NULL),_wfname(NULL),
		_sgfname(NULL),_ERROR_MES(NULL),
		_ub(INTHUGE),_lb(0),
		_prob_ub(1),_prob_lb(0),
		_target(INTHUGE),
		_ratio_lb(0),_ratio_ub(1),
		_nega_lb(-WEIGHTHUGE),_nega_ub(WEIGHTHUGE),
		_posi_lb(-WEIGHTHUGE),_posi_ub(WEIGHTHUGE),
		_rposi_lb(0),_rposi_ub(1),_max_solutions(0),
		_separator(' '),_digits(4),
		_frq_lb(-WEIGHTHUGE),_frq_ub(WEIGHTHUGE),_topk_k(0){};




	int run (int argc, char *argv[]);
	

};

