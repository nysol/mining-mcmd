/* take the intersection of each cluster */
/* 8/Nov/2008   Takeaki Uno  */

#include"fstar.hpp"
#include"stdlib2.hpp"
#include"medset.hpp"
#include"problem.hpp"



void MEDSET::_error (){
  _ERROR_MES = "command explanation";
  print_err ("medset: compute the intersection of each set of sets\n\
usage: medset [-HRTIitl] cluster-filename set-filename threshold output-filename\n\
if threshold is negative, output the items whose frequencies are no more than the threshold\n\
-%%: show progress, -_: no message\n\
-H: do not use histgram, just output the items\n\
-R: do not output singleton clusters\n\
-V: output ratio of appearances of all items\n\
-T: clustering by connected component (read edge type file)\n\
-I: find an independent set, and clustering by using the vertices in it as seeds (read edge type files)\n\
-i: output for each item, ratio of records including the item\n\
-t: transpose the input database, (transaction will be item, and vice varsa)\n\
-l [num]: output clusters of size at least [num]\n\
# the 1st letter of input-filename cannot be '-'.\n");
  EXIT;
}

/* read commands and options from command line */
void MEDSET::read_param (int argc, char *argv[]){
  int c=1;

  if ( argc < c+3 ){ _error (); return; }
  _dir = 1; _FS.union_flag (SHOW_MESSAGE);
  while ( argv[c][0] == '-' ){
    if ( argc<c+3 ){ _error (); return; }
    switch ( argv[c][1] ){
      case 't': _FS.union_flag ( LOAD_TPOSE);
      break; case '_': _FS.sub_flag (SHOW_MESSAGE);   // connected component clustering
      break; case '%': _FS.union_flag(SHOW_PROGRESS);   // connected component clustering
      break; case 'T': _problem |= MEDSET_CC;   // connected component clustering
      break; case 'I': _problem |= MEDSET_IND;   // independent set clustering
      break; case 'H': _problem |= MEDSET_NO_HIST;   // do not use histgram
      break; case 'V': _problem |= MEDSET_ALLNUM;   // output appearance ratio for all
      break; case 'l': _num = atoi(argv[c+1]); c++;   // minimum cluster size to be output
      break; case 'i': _problem |= MEDSET_RATIO;   // output included-ratio of items
//      break; case 'c': _deg = atoi(argv[c+1]); c++;   // least degree
   }
    c++;
  }
  
  _input_fname = argv[c];
  if ( !(_problem & (MEDSET_CC+MEDSET_IND))) _FS.set_fname(argv[c+1]);
  _th = atof(argv[c+2]);
  if ( _th < 0 ){ _th = -_th; _problem |= MEDSET_BELOW; }  // output less frequency items
  _output_fname = argv[c+3];
}



/* read file, output the histogram of each line */
void MEDSET::read_file (FILE2 *fp){

  FSTAR_INT *cnt, *que, t, s, i, x;

  calloc2 (cnt, _FS.get_in_node_num(), EXIT);
  calloc2 (que, _FS.get_in_node_num()*2, goto END);
  
  do {
    s = t = 0;
    do {   // count #out-going edges for each vertex
      x = (FSTAR_INT)fp->read_int ();
      if ( FILE_err&4 ) break;
      if ( x<0 || x >= _FS.get_out_node_num() ){
        print_err ("set ID out of bound "FSTAR_INTF">"FSTAR_INTF"\n", x, _FS.get_out_node_num());
        exit(0);
      }
      FLOOP (i, _FS.get_fstar(x), _FS.get_fstar(x+1))
          if ( cnt[_FS.get_edge(i)]++ == 0 ){ que[t*2+1] = _FS.get_edge(i); t++; }
      s++;
    } while ( (FILE_err&3)==0 );

    if ( _problem & MEDSET_ALLNUM ){
      FLOOP (i, 0, _FS.get_in_node_num()){
        fprintf (_ofp, "%.2f ", ((double)cnt[i])/(double)s);
        cnt[i] = 0;
      }
      fprintf (_ofp, "\n");
      continue;
    }

    if ( s>0 ){
      FLOOP (i, 0, t){ que[i*2] = cnt[que[i*2+1]]; cnt[que[i*2+1]] = 0; }
      qsort_<FSTAR_INT> (que, t, (_problem&MEDSET_BELOW?1:-1)*((int)sizeof(FSTAR_INT))*2);

      FLOOP (i, 0, t){
        if ( _problem & MEDSET_BELOW ){
           if ( ((double)que[i*2])/(double)s > _th ) break;
        } else if ( ((double)que[i*2])/(double)s < _th ) break;
        if ( _problem & MEDSET_NO_HIST ) fprintf (_ofp, ""FSTAR_INTF" ", que[i*2+1]);
        else if ( _problem & MEDSET_RATIO ) fprintf (_ofp, "("FSTAR_INTF":%.2f) ", que[i*2+1], ((double)que[i*2])/(double)s);
        else fprintf (_ofp, ""FSTAR_INTF" ", que[i*2+1]);
      }
    }
    fprintf (_ofp, "\n");
  } while ( (FILE_err&2)==0 );

  END:;
  mfree (cnt, que);
}


/* output clusters to the output file */
void MEDSET::print_clusters (FSTAR_INT *mark, FSTAR_INT *set, FSTAR_INT xmax){
  FSTAR_INT i, x, c;
  
  FLOOP (i, 0, xmax){
    if ( mark[i] != i ) continue;
    c = 0; x = i;
    while (1){
      c++;
      if ( set[x] == x ) break;
      x = set[x];
    }
    if ( c < _num ) continue;
    x = i;
    while (1){
      fprintf (_ofp, ""FSTAR_INTF" ", x);
      if ( set[x] == x ) break;
      x = set[x];
    }
    fputs ("\n", _ofp);
  }
}


/* read file, output the histogram of each line */
void MEDSET::cc_clustering (FILE2 *fp){
  FSTAR_INT *pnt=NULL, end1=0, end2=0, xmax=0, *mark=NULL, *set=NULL;
  LONG x, y;
    // merge the connponents to be connected by using spray tree
  do {
    if ( fp->read_pair ( &x, &y, NULL, 0) ) continue;
    ENMAX (xmax, MAX(x, y)+1);
    reallocx (mark, end1, xmax, common_size_t, EXIT);
    reallocx (set, end2, xmax, common_size_t, EXIT);
    UNIONFIND_unify_set (x, y, (UNIONFIND_ID *)mark, (UNIONFIND_ID *)set);
  } while ( (FILE_err&2)==0 );

  print_clusters ( mark, set, xmax);

  END:;
  mfree (mark, set);
}


/* clustering the nodes by finding independent set */
/* cnt: cluster siz, if v is representative, and #vertices covering v, if v isn't representative */
void MEDSET::ind_clustering (FILE2 *fp){
  FSTAR_INT *pnt=NULL, flag, end1=0, end2=0, end3=0, xmax=0, *mark=NULL, *set=NULL, *cnt=NULL;
  LONG x, y, yy;

    // merge the connponents to be connected by using spray tree
  do {
    flag = 0;
    do {
      if ( fp->read_pair ( &x, &y, NULL, 0) ) continue;
      ENMAX (xmax, MAX(x, y)+1);
      reallocx (mark, end1, xmax, common_size_t, EXIT);
      reallocx (set, end2, xmax, common_size_t, EXIT);
      reallocx (cnt, end3, xmax, 0, EXIT);
      if ( cnt[x] < cnt[y] ) SWAP_<LONG> (&x, &y);
      if ( mark[x] == x && mark[y] == y ){
        if ( set[x] == x && !(set[y]== y && cnt[y]>0) ){ 
        	UNIONFIND_unify_set (y, x, (UNIONFIND_ID *)mark, (UNIONFIND_ID *)set); 
        	cnt[y]++; 
        	cnt[x] = 1; 
        	flag = 1; 
        }
        else {
          do {
            yy = set[y];
            set[y] = y;
            y = yy;
            mark[y] = y;
            cnt[y]--;
          } while (y != set[y]);
        }
        if ( set[y] == y ){ 
        	UNIONFIND_unify_set (x, y, (UNIONFIND_ID *)mark, (UNIONFIND_ID *)set); 
        	cnt[x]++; 
        	cnt[y] = 1; 
        	flag = 1;
        }
      }
      if ( mark[x] == x ){ cnt[y]++; }
      else if ( mark[y] == y ){ cnt[x]++; }
    } while ( (FILE_err&2)==0 );
  } while (flag);

  print_clusters (mark, set, xmax);

  END:;
  mfree (mark, set, cnt);
}


/*******************************************************************/
int MEDSET::run (int argc, char *argv[]){

  FILE2 fp;
  
  //PROBLEM_init (&PP);

  read_param ( argc, argv);

	if ( _ERROR_MES ) return (1);

  _FS.union_flag ( LOAD_BIPARTITE);
  _FS.set_edge_dir(1);

	preLOAD();

  print_mesf (&_FS, "medset: cluster-file= %s set-file= %s threshold= %f output-file= %s\n", _input_fname, _FS.get_fname(), _th, _output_fname);

  //PROBLEM_load (&PP);
  
  fp.open ( _input_fname, "r");
  fopen2 (_ofp, _output_fname, "w", goto END);

  if ( !_ERROR_MES ){
    if ( _problem & MEDSET_CC ) cc_clustering (&fp);
    else if ( _problem & MEDSET_IND ) ind_clustering ( &fp);
    else read_file (&fp);
  }

  END:;
  fp.close();
  fclose2 (_ofp);

//  PROBLEM_end (&PP);
  return (ERROR_MES?1:0);
}
int MEDSET::mrun (int argc, char *argv[]){
	return MEDSET().run(argc,argv);
}

