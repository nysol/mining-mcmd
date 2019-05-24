/* library for standard macros and functions 
 by Takeaki Uno 2/22/2002   e-mail: uno@nii.jp
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#include "trsact.hpp"
#include "filecount.hpp"



QUEUE_INT FILE_COUNT::_weight_Scan(char *wf){

	FILE2 wfp;
	wfp.open(wf, "r");

	#ifdef WEIGHT_DOUBLE
		QUEUE_INT kk = wfp.ARY_Scan_DBL(1);
	#else
		QUEUE_INT kk = wfp.ARY_Scan_INT(1);
	#endif

	kk += _rows_org;
	_rw.realloc2(kk+1);

	wfp.reset();
	wfp.VARY_Read(_rw, kk);
			
	if ( _rw.min(0, kk) < 0 ) {
		_negaFLG = true;
	}

	wfp.close();
	
	return kk;
	
}


/*****************************************/
/* scan file "fp" with weight file wfp and count #items, #transactions in the file. */
/*   count weight only if wfp!=NULL                                      */
/* T->rows_org, clms_org, eles_org := #items, #transactions, #all items  */
/*   ignore the transactions of size not in range T->clm_lb - clm_ub     */ 
/* T->total_w, total_pw := sum of (positive) weights of transactions     */
/* C->clmt[i],C->cw[i] := the number/(sum of weights) of transactions including i  */
/****************************************/
// LOAD_TPOSEの時
int FILE_COUNT::_file_count_T(FILE2 &fp,char *wf){

	  QUEUE_INT i, item, kk=0, k, jump_end=0;
	  WEIGHT w, s;
	  VECARY<VEC_ID> jump;
  	LONG jj;

	  if ( wf ){ kk = _weight_Scan(wf); }

	  do {

  	  s=0; k=0;

    	w = wf? (_rows_org<kk? _rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;

    	do {

      	jj = fp.read_int();
      	item = (QUEUE_INT)jj;

				if ( fp.readOK() && jj<TRSACT_MAXNUM && jj>=0 ){ //(FILE_err&4)==0

					// update #items
       		ENMAX (_clms_org, item+1);  
        	jump_end = jump.reallocx(jump_end, k, 0);

      		jump[k] = item;
        	k++;
        	s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

          // count/weight-sum for the transpose mode
        	_clm_end = _clmt.reallocx(_clm_end, item, 0);

        	_clmt[item]++;
	      }

  		} while ( fp.remain() );

       // count/weight-sum for the transpose mode
			_row_end = _rowt.reallocx(_row_end, _rows_org, 0);
    	_rowt[_rows_org] = k;

			// LOAD_TPOSEの時
			_cw_end = _cw.reallocx(_cw_end, _rows_org, 0);

      _cw[_rows_org] = s;    // sum up positive weights

			if ( k==0 && fp.eofx() ) break;
			_rows_org++;  // increase #transaction

    	
			if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

			_eles_org += k;

			// LOAD_TPOSEの時はこの条件
	    if ( !RANGE( _w_lb, s, _w_ub) || !RANGE (_clm_lb, k, _clm_ub)  ){
				for(int i0 = 0 ; i0 < k ; i0++){
					_clmt[jump[i0]]--; 
				}
      }


		} while ( fp.eof() );

		//delete [] jump
    // swap the variables in transpose mode
  	if ( _rw.empty() ){
  		_total_w_org = _total_pw_org = _rows_org; 
  		return 0; 
  	} 

		_clm_btm = MIN(kk, _rows_org);

		kk = _rw.reallocx(kk,_rows_org, TRSACT_DEFAULT_WEIGHT);

		for(int i0 = 0 ; i0 < _rows_org ; i0++){
  	  _total_w_org  += _rw[i0];
    	_total_pw_org += MAX(_rw[i0],0);
	  }
	  return 0;
  
}



// NOT LOAD_TPOSEの時
int FILE_COUNT::_file_count(FILE2 &fp, char *wf){
	
  QUEUE_INT i, item, kk=0, k, jump_end=0;
  WEIGHT w, s;
  VECARY<VEC_ID> jump;

	LONG jj;

	if ( wf ){ kk = _weight_Scan(wf); }


	do {

		s=0; k=0;
		w = wf? (_rows_org<kk? _rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;

		do {
			
			jj = fp.read_int();
			item = (QUEUE_INT)jj;

			if ( fp.readOK() && jj<TRSACT_MAXNUM && jj>=0 ){

				ENMAX (_clms_org, item+1);  // update #items
				jump_end = jump.reallocx(jump_end, k, 0);

				jump[k] = item;
				k++;
				s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

				// count/weight-sum for the transpose mode
				_clm_end = _clmt.reallocx(_clm_end, item, 0);
				_clmt[item]++;

				// NOT TPOSE
				_cw_end = _cw.reallocx(_cw_end, item, 0);
				_cw[item] += MAX(w,0);    // sum up positive weights

			}

		} while ( fp.remain());

		// count/weight-sum for the transpose mode
		_row_end = _rowt.reallocx (_row_end, _rows_org, 0);

		_rowt[_rows_org] = k;

		if ( k==0 && fp.eofx() ) break;

		_rows_org++;  // increase #transaction
    
		if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

		_eles_org += k;

		// NOT LOAD_TPOSEの時はこの条件
		if( !RANGE (_row_lb, k, _row_ub) ){
			for(int i0=0 ; i0 < k ; i0++ ){
				_clmt[jump[i0]]--; 
			}
		}

	} while ( fp.eof() );


	// swap the variables in transpose mode
	if ( _rw.empty() ){
		_total_w_org = _total_pw_org = _rows_org; 
		return 0; 
	} 

	_clm_btm = MIN(kk, _rows_org);
	kk = _rw.reallocx( kk, _rows_org, TRSACT_DEFAULT_WEIGHT);


	for(int i0=0 ; i0 < _rows_org ; i0++ ){
		_total_w_org += _rw[i0];
		_total_pw_org += MAX(_rw[i0],0);
	}
	
	return 0;

}


int FILE_COUNT::file_count(int flg, FILE2 &fp, FILE2 &fp2, char *wf){

	if(flg){ // TPOSE

		if( _file_count_T(fp, wf) ) { return 1; }	
		_end1 = _rows_org;

		if( fp2.exist() ){
			if( _file_count_T ( fp2, NULL) ) { return 1; }
		}
		// swap variables in the case of transpose
		_tpose();

	} 
	else{
		if( _file_count( fp, wf) ){ return 1;	}
		_end1 = _rows_org;

		if( fp2.exist() ){
			if( _file_count(fp2, NULL) ) { return 1;	}
		}

	}

	_setBoundsbyRate();

	return 0;

}


void FILE_COUNT::count( 
		FILE2 *rfp,
		int flag, int skip_rows,
		int int_rows, int skip_clms, int int_clms, 
	 	FILE_COUNT_INT row_limit
)
{
	  FILE_COUNT_INT k=0, j, x, y, t=0;

  	// flags for using rowt, and clmt, that counts elements in each row/column
  	int fr = flag&FILE_COUNT_ROWT, fc = flag&FILE_COUNT_CLMT; 

		// fe,ft: flag for ele mode, and transposition
	  int fe = flag&LOAD_ELE, ft = flag&LOAD_TPOSE;  

  	//_flag = flag;
		for(int i0=0 ; i0 <skip_rows ; i0++){
	  	 rfp->read_until_newline();
	  }

	  if ( flag & (FILE_COUNT_NUM+FILE_COUNT_GRAPHNUM) ){

  	  _clms = (FILE_COUNT_INT) rfp->read_int ();
    	_rows = (flag & FILE_COUNT_NUM)? (FILE_COUNT_INT) rfp->read_int(): _clms;
    	_eles = (FILE_COUNT_INT) rfp->read_int();

	    if ( !(flag & (FILE_COUNT_ROWT + FILE_COUNT_CLMT)) ) return ;
  	  rfp->read_until_newline ();
		}

	  do {
	    if ( fe ){
			
				for(int i0=0 ; i0 <skip_clms ; i0++){

					rfp->read_double (); 
					if ( rfp->getOK()) goto ROW_END;  // FILE_err&3 ここあってる？

				}
			
				x = (FILE_COUNT_INT) rfp->read_int ();
				if ( rfp->getOK() ) goto ROW_END; //  FILE_err&3 ここあってる？
			
      	y = (FILE_COUNT_INT) rfp->read_int (); 
				if ( rfp->readNG() ) goto ROW_END; //  FILE_err&4 ここあってる？

      	rfp->read_until_newline ();
    	}
    	else 
    	{
      	if ( k==0 ) {
					for(int i0=0 ; i0 <skip_clms ; i0++){
      			rfp->read_double (); 
      			if (rfp->getOK()) goto ROW_END;  //FILE_err&3
      		}
      	}
				x = t;
      	y = (FILE_COUNT_INT)rfp->read_int (); 
				if ( rfp->readNG() ) goto ROW_END; // FILE_err&4  ここあってる？

				for(int i0=0 ; i0 <int_clms ; i0++){
      		rfp->read_double (); 
      		if ( rfp->getOK() ) break; //FILE_err&3
      	}
				k++;
    	}
    	
	    if ( ft ){
	    	SWAP_<FILE_COUNT_INT>(&x, &y);
	    }

	    if ( y >= _clms ){
  	    _clms = y+1;
    	  if ( fc ) {
    	  	_clm_end = _clmt.reallocx(_clm_end, _clms, 0);
    	  }
			}
			
			if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && x >= _clms ){
      	_clms = x+1;
				if ( fc ) { 
					_clm_end = _clmt.reallocx ( _clm_end, _clms, 0);
				}
	    }
 
 			if ( x >= _rows ){
      	_rows = x+1;
				if ( fr ) { 
					_row_end = _rowt.reallocx(_row_end, _rows, 0);
				}
			}
		
			if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && y >= _rows ){ // for undirected edge version
      	_rows = y+1;
	      if ( fr ) { 
	      	_row_end = _rowt.reallocx(_row_end, _rows, 0);
	      }
    }
    
    if ( x < _clm_btm || _eles == 0 ) {  _clm_btm = x; }
    if ( y < _row_btm || _eles == 0 ) {  _row_btm = y; }
    if ( fc ) { _clmt[y]++; }
    if ( fr ){ 
    	_rowt[x]++; 
    	if ( flag&LOAD_EDGE && x != y ){ _rowt[y]++; }
    }	
    
    _eles++;

    ROW_END:;

    //if ( !fe && (FILE_err&1) ){
    if ( !fe && rfp->getOK1() ){
      t++;

      if ( flag&(LOAD_RC_SAME+LOAD_EDGE) ){
        ENMAX (_clms, t); ENMAX (_rows, t);
      } 
      else if ( ft ) {	
      	_clms = t;
      } 
      else { 
      	_rows = t;
      }

      ENMAX (_clm_max, k);
      ENMIN (_clm_min, k);

			for(int i0=0 ; i0 <int_rows ; i0++){
      	rfp->read_until_newline ();
      }
      if ( row_limit>0 && t>=row_limit ) { break; }
    } 
    else if ( row_limit > 0 && _eles>=row_limit ) {
    	break;
    }

  } while ( rfp->eof());

  if ( fc ){ 
  	_clm_end = _clmt.reallocx(_clm_end, _clms, 0); 
  }
  if ( fr ){
    _row_end = _rowt.reallocx(_row_end, _rows, 0);
    // 一緒にする？
    _row_max = _rowt.max(0, _rows);
		_row_min = _rowt.min(0, _rows);
		
  }
  if ( fe && !_clmt.empty() ){
    // 一緒にする？
    _clm_max = _clmt.max(0, _clms);
    _clm_min = _clmt.min(0, _clms);
  }
  END:;

  return ;

}

//void FILE_COUNT::initCperm(VEC_ID ttt , PERM *p ,QUEUE_INT c_end , bool flag){

// pfname :filename
// wflg : true:write to pfname file   false: read from pfname file

// このクラス_crpermはいらない？
void FILE_COUNT::initCperm(char *pfname,int tflag,int tflag2){

	// ttt :perm Size 
	VEC_ID ttt_max = _clms_org;
	VEC_ID ttt = _clms_org;
  PERM *p=NULL;
 // PERM *cperm=NULL;

	// この分岐を整理すること。わける
  if ( pfname && !( tflag2&TRSACT_WRITE_PERM) ){ 

		ttt = FILE2::ARY_Load(p, pfname, 1);
		ttt_max = p[0];
		for(int i=1; i < ttt ; i++){
			if(ttt_max < p[i]){ ttt_max = p[i]; }
		}
  }
  else {
  	// LOAD_PERM TRSACT_FRQSORT  デフォルトでセットされる(LCM) //この辺りも分ける？
    // LOAD_INCSORTはセットされない(sgflagにセットされるかのうせいあり)
    if ( tflag&LOAD_PERM ){ 
      if ( tflag2&TRSACT_FRQSORT ){ 
      	p = clmw_perm_sort((tflag&LOAD_INCSORT)?1:-1);
      }
      else {
      	p = clmt_perm_sort((tflag&LOAD_INCSORT)?1:-1);
      }
    }

    if ( pfname ) { 
    	FILE2::ARY_Write(pfname , p , _clms_org); 
    }
  }
	
	_clms_end = MAX(_clms_org, ttt_max);
	_c_eles=0;
	_c_clms=0;

	_cperm = new PERM[_clms_org+1]; // malloc2
	for(size_t i =0 ;i<_clms_org;i++){ 
		_cperm[i] = _clms_org+1; 
	}

	VEC_ID tt =0 ;
	for(size_t t=0; t < ttt; t++){
		tt = p? p[t]: t;
		if ( tt >= _clms_org ) continue;
		    
		if ( RangeChecnkC(tt) ){
			_c_eles += _clmt[tt];
			_cperm[tt] = (  pfname && !(tflag2&TRSACT_WRITE_PERM) )  ? t : _c_clms++ ;
		}
		else{
			_cperm[tt] = _clms_end+1 ;
		}
	}

	if( pfname && !(tflag2&TRSACT_WRITE_PERM) ){
		_c_clms = ttt_max+1;
	}
	delete [] p;
	return ;
}


//void FILE_COUNT::initRperm(PERM *p , size_t base_clm, size_t base_ele){
void FILE_COUNT::initRperm(int tflag){

  PERM *p=NULL;

  if ( tflag&(LOAD_SIZSORT+LOAD_WSORT) ){
    if ( tflag&LOAD_WSORT ){
    	// p =_C.roww_perm_sort((tflag&LOAD_DECROWSORT)?-1:1);
    	p = roww_perm_sort((tflag&LOAD_DECROWSORT)?-1:1);

    }	else {
      //p =_C.rowt_perm_sort((tflag&LOAD_DECROWSORT)?-1:1);
      p = rowt_perm_sort((tflag&LOAD_DECROWSORT)?-1:1);
    }
  }
 
	// _r_eles = base_ele;  _r_clms = base_clm;

	_rperm = new PERM[_rows_org];//malloc2
			
	// compute #elements according to rowt, and set rperm
	VEC_ID tt=0;
	for( VEC_ID t=0 ; t<_rows_org ; t++){
		tt = p? p[t]: t; 
		if ( RangeChecnkR(tt)){
			_rperm[tt] = _r_clms++;
			_r_eles += _rowt[tt];
		}
		else{
			_rperm[tt] = _rows_org+1;
		}
	}
	return;
}
	
void FILE_COUNT::makePerm(char *pfname,int tflag,int tflag2){

//	PERM ** rtn = new PERM *[2]:

	initCperm(pfname,tflag,tflag2);

	if ( _c_clms == 0 ) throw ("there is no frequent item");

	//複数回動くことを考えてる？ おそらく両方0
	//initRperm( p , _T.get_eles(), _T.get_t()); 
	initRperm( tflag ); 
	
	
	return ;
	

}



/*

void FILE_COUNT::initCperm(VEC_ID ttt , PERM *p ,QUEUE_INT c_end , bool flag){

	_c_eles=0;
	_c_clms=0;

	_cperm = new PERM[_clms_org+1]; // malloc2
	for(size_t i =0 ;i<_clms_org;i++){ 
		_cperm[i] = _clms_org+1; 
	}

	VEC_ID tt =0 ;
	for(size_t t=0; t < ttt; t++){
		tt = p? p[t]: t;
		if ( tt >= _clms_org ) continue;
		    
		if ( RangeChecnkC(tt) ){
			_c_eles += _clmt[tt];
			_cperm[tt] = flag ? t : _c_clms++ ;
		}
		else{
			_cperm[tt] = c_end ;
		}
	}
	return ;
}

void FILE_COUNT::initRperm(PERM *p , size_t base_clm, size_t base_ele){

			_r_eles = base_ele;
			_r_clms = base_clm;

			//malloc2 (_rperm, _rows_org, exit(1));
			_rperm = new PERM[_rows_org];
			
		  // compute #elements according to rowt, and set rperm
			VEC_ID tt=0;
			for( VEC_ID t=0 ; t<_rows_org ; t++){
				tt = p? p[t]: t;
		    if ( RangeChecnkR(tt)){
			    _rperm[tt] = _r_clms++;
    		  _r_eles += _rowt[tt];
    		}
    		else{
					_rperm[tt] = _rows_org+1;
    		}
			}
}


*/










