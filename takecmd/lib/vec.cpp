/* library for vector and sparse vector, and matrix */
/* Takeaki Uno    27/Dec/2008 */

//#ifndef _vec_c_
//#define _vec_c_

#include"vec.hpp"
#include"stdlib2.hpp"
#include"queue.hpp"
#include"filecount.hpp"

/* allocate memory according to rows and rowt */
/* if eles == 0, compute eles from rowt and rows */
void SETFAMILY::alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){

  VEC_ID i;
  char *buf;
  if ( eles == 0 ) ARY_SUM (_ele_end, rowt, 0, rows);
  else _ele_end = eles;

  calloc2 (buf, (_ele_end*((_flag&LOAD_DBLBUF)?2:1) +(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)*_unit, EXIT);
  _buf = (QUEUE_INT *)buf;
  
  malloc2 (_v, rows+1, {free(_buf);EXIT;});

	for(size_t i =0 ;i<rows;i++){ 
		_v[i] = QUEUE(); 
	}

  _end = rows;
  _clms = clms;

  if ( rowt ){
    FLOOP (i, 0, rows){
      _v[i].set_v((QUEUE_INT *)buf);
      buf += (rowt[i] +1)*_unit;
      _v[i].set_end(rowt[i]+1);
    }
  }
}

/* allocate memory according to rows and rowt */
/* if eles == 0, compute eles from rowt and rows */
void SETFAMILY::alloc_weight ( QUEUE_ID *t){
  VEC_ID i;
  calloc2 (_w, _end +1, EXIT);
  calloc2 (_wbuf, _ele_end*((_flag&LOAD_DBLBUF)?2:1)+1, {free(_w);EXIT;});
  _w[0] = _wbuf; FLOOP (i, 1, _t) _w[i] = _w[i-1] + (t?t[i-1]:_v[i-1].get_t());
}

void SETFAMILY::alloc_w (){
  calloc2 (_w, _end, EXIT);
}


/* sort and duplication check */
void SETFAMILY::sort (){
  VEC_ID i;
  PERM *p;
  WEIGHT *ww, w;
  QUEUE Q;

  int flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);
  // sort items in each row
  if ( flag ){   
    malloc2 (p, _clms, EXIT);
    FLOOP (i, 0, _t)
      _v[i].perm_WEIGHT( _w?_w[i]:NULL, p, flag);
    free (p);
  }

  flag = ((_flag&(LOAD_SIZSORT+LOAD_WSORT))? ((_flag&LOAD_DECROWSORT)? -1: 1): 0);
  if ( flag ){   // sort the rows
    //if ( _flag & LOAD_SIZSORT ) p = qsort_perm_<VEC> ((VEC *)_v, _t, flag*sizeof(QUEUE));
		if ( _flag & LOAD_SIZSORT ) p = qsort_perm_<QUEUE> (_v, _t, flag*sizeof(QUEUE));
    else p = qsort_perm_<WEIGHT> (_rw, _t, flag*sizeof(WEIGHT));


		malloc2(_rperm,_t,EXIT);
		for(size_t st=0; st<_t ;st++){ _rperm[st]=-1; }

		for(int i=0;i<_t;i++){
			if(p[i]>=0 && p[i]<_t){ _rperm[p[i]]=i; }
		}

    if ( _rw ){
			any_INVPERMUTE_rw(p);
    	//ARY_INVPERMUTE(_rw, p, w, _t, EXIT);
    }
    if ( _w ){
	    any_INVPERMUTE_w(p);
    	// ARY_INVPERMUTE(_w, p, ww, _t, EXIT);
    }
    
    ary_INVPERMUTE_(p,Q);
    //ARY_INVPERMUTE_(_v, p, Q, _t);


    free2 (p);
  }

  if (_flag&LOAD_RM_DUP){  // unify the duplicated edges
    FLOOP (i, 0, _t){
			_v[i].rm_dup_WEIGHT( _w?_w[i]:NULL);
		}
  }
}

void SETFAMILY::SMAT_alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){
  VEC_ID i;
  if ( eles == 0 ) ARY_SUM (_ele_end, rowt, 0, rows); else _ele_end = eles;
  calloc2 (_buf, _ele_end*((_flag&LOAD_DBLBUF)?2:1) +rows +2, EXIT);
  malloc2 (_v, rows+1, {free(_buf);EXIT;});
  //ARY_FILL (_v, 0, rows, SVEC() );

	for(size_t _common_size_t =0;_common_size_t<rows;_common_size_t++){
		_v[_common_size_t].set_end(0);
	  _v[_common_size_t].set_t(0);
	}

  _end = rows;
  _clms = clms;
  if ( rowt ){
    FLOOP (i, 0, rows){
      _v[i].set_v( i? _v[i-1].get_v() + rowt[i-1] +1: _buf);
      _v[i].set_end( rowt[i]);
    }
  }
}


void SETFAMILY::flie_load(FILE2 *fp){

  WEIGHT z=0;
  VEC_ID t;
  LONG x, y;
  int fc=0, FILE_err_=0, flag2=_flag;
  int flag=0;

  int wflag =  ( _wfname || (_flag&LOAD_EDGEW));
  FILE_COUNT C;
  FILE2 wfp;

  if ( flag && !_wfname ) flag2 = _flag | LOAD_EDGEW;

  C.count(fp,(_flag&(LOAD_ELE+LOAD_TPOSE+LOAD_RC_SAME+LOAD_EDGE)) | FILE_COUNT_ROWT, 0, 0, 0, (flag2&LOAD_EDGEW)?1:0, 0);

  if ( _clms == 0 ) _clms = C.get_clms();
  if ( _t == 0 ) _t = C.get_rows();
  if ( flag ) SMAT_alloc ( _t, C.getp_rowt(), _clms, 0);
  else {
    alloc ( _t, C.getp_rowt(), _clms, 0);
    if ( wflag ) alloc_weight ( C.getp_rowt());
  }

  if ( _wfname ) wfp.open ( _wfname, "r");
  if ( ERROR_MES ) return;

  fp->reset ();
  if ( _flag&(LOAD_NUM+LOAD_GRAPHNUM) ) fp->read_until_newline ();
  t=0;
  do {
    if ( _flag&LOAD_ELE ){
      if ( fp->read_pair ( &x, &y, &z, flag2) ) continue;
    } else {
      x = t;
      FILE_err_ = fp->read_item ( _wfname?&wfp:NULL, &x, &y, &z, fc, flag2);
      if ( FILE_err&4 ) goto LOOP_END;
    }

    if ( y >= _clms || x >= _t ) continue;

    if ( flag ){
      _v[x].inc_t();
    } else {
      if ( wflag ) _w[x][_v[x].get_t()] = z;
      _v[x].push_back(y);
      if ( (_flag&LOAD_EDGE) && x != y ){
        if ( wflag ) _w[y][_v[y].get_t()] = z;
        _v[y].push_back(x);
      }
    }
    if ( !(_flag&LOAD_ELE) ){
      fc = 0;
      if ( FILE_err&3 ){
        LOOP_END:;
        t++; if ( t >= _t ) break;
        fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
      }
    }
  } while ( (FILE_err&2)==0 );
  if ( _wfname ) wfp.close ();

}


/* scan file and load the data from file to SMAT structure */
void SETFAMILY::load (int flag , char *fname)
{

  FILE2 fp;
  VEC_ID i;
  _fname = fname;
  _flag = flag;

  fp.open ( _fname, "r");
	flie_load(&fp);
  fp.close ();     

  if(ERROR_MES) EXIT;

  print_mes (this, "setfamily: %s ,#rows %d ,#clms %d ,#eles %zd", _fname, _t, _clms, _eles);

  if (_wfname ) print_mes (this, " ,weightfile %s", _wfname);
  print_mes (this, "\n");
 
  sort ();

	// end mark これいる？
  FLOOP (i, 0, _t) _v[i].set_v(_v[i].get_t(),_clms); 

  _eles = _ele_end;

	for(int i=0 ; i< _t;i++){ // allvvInitByT();
		_v[i].set_v( _v[i].get_t() , _t);
	}

  sort(); //これいる？

}

void SETFAMILY::replace_index(PERM *perm, PERM *invperm){

		  if ( _v ){
		  	for(size_t i=0; i<_t ; i++){
			  	for( QUEUE_INT *x = _v[i].begin() ; x < _v[i].end() ; x++ ){
  					*x = perm[*x];
					}
    		}
		    // INVPERMUTE
				char * cmmp; 
			  QUEUE Q;
			  int i1,i2;
				calloc2(cmmp,_t,EXIT);
				for( i1 = 0; i1 < _t ; i1++ ){
					if ( cmmp[i1]==0 ){ 
						Q = _v[i1]; 
						do{ 
							i2 = i1; 
							i1 = invperm[common_INT]; 
							_v[i2]=_v[i1]; 
							cmmp[i2] = 1;
						} while( cmmp[i1]==0 );
						_v[i2] = Q; 
					}
				}
				free(cmmp);
    	}

		  if ( _w ){
		    // INVPERMUTE
				char * cmmp; 
 				WEIGHT *w;
 			  int i1,i2;
				calloc2(cmmp,_t,EXIT);
				for( i1 = 0; i1 < _t ; i1++ ){
					if ( cmmp[i1]==0 ){ 
						w = _w[i1]; 
						do{ 
							i2 = i1; 
							i1 = invperm[common_INT]; 
							_w[i2]=_w[i1]; 
							cmmp[i2] = 1;
						} while( cmmp[i1]==0 );
						_w[i2] = w; 
					}
				}
				free(cmmp);
		  }

}


void SETFAMILY::setInvPermute(PERM *rperm,PERM *trperm,int flag){

			QUEUE Q;	
			char  *cmm_p;
			int cmm_i,cmm_i2;

			qsort_perm__( _v, _t, rperm, flag); 

			calloc2(cmm_p,_t,EXIT);

			FLOOP(cmm_i,0,_t){ 
				if ( cmm_p[cmm_i]==0 ){ 
					Q = _v[cmm_i]; 
					do{ 
						cmm_i2=cmm_i; 
						cmm_i=rperm[cmm_i]; 
						_v[cmm_i2]=_v[cmm_i]; 
						cmm_p[cmm_i2]=1; 
					}while( cmm_p[cmm_i]==0 ); 
					_v[cmm_i2] = Q; 
				}
			}
			free(cmm_p); 
			
			if(_w){
				WEIGHT *ww;
				calloc2(cmm_p,_t,EXIT);
				FLOOP(cmm_i,0,_t){ 
					if ( cmm_p[cmm_i]==0 ){ 
						ww = _w[cmm_i]; 
						do{ 
							cmm_i2=cmm_i; 
							cmm_i=rperm[cmm_i]; 
							_w[cmm_i2]=_w[cmm_i]; 
							cmm_p[cmm_i2]=1; 
						}while( cmm_p[cmm_i]==0 ); 
						_w[cmm_i2] = (ww); 
					}
				}
				free(cmm_p); 
			} 
			
		  PERM pp;
			FLOOP(cmm_i,0,_t){ 
				if ( rperm[cmm_i]< _t ){ 
					pp = trperm[cmm_i]; 
					do { 
						cmm_i2=cmm_i; 
						cmm_i=rperm[cmm_i]; 
						rperm[cmm_i2]=trperm[cmm_i]; 
						rperm[cmm_i2]=_t; 
					}while ( rperm[cmm_i]< _t ); 
					rperm[cmm_i2] = pp;
				}
			}
}
