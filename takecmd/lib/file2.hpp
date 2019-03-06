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
#pragma once

#define WEIGHT_DOUBLE

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include<stdarg.h>
#include"stdlib2.hpp"
//#include"filecount.hpp"

class FILE2 {   // structure for fast file reader routines

  FILE *_fp;
  char *_buf_org, *_buf, *_buf_end;   // head/current/tail of buffer
  char _bit;

	public :
		FILE2():
		_fp(NULL),_buf_org(NULL),_buf(NULL)
		,_buf_end(NULL),_bit(0){}
		
		void open(char *fname,char *rw) 
		{
			if(fname){ fopen2( _fp,fname,rw,EXIT);}
			else { _fp=NULL;}
			
			malloc2(_buf_org,FILE2_BUFSIZ+1,EXIT);
			_buf=_buf_org;
			_buf_end=_buf_org-1;
			_bit=0;
			*_buf=0;
		}
		void open_(FILE *a) {
			_fp=a;
			malloc2(_buf_org,FILE2_BUFSIZ+1,EXIT);
			_buf=_buf_org;
			_buf_end=_buf_org-1;
			_bit=0;
			*_buf=0;
		}
		bool exist_buf(){ return _buf!=NULL; }
		void flush_last (void);
		void flush_ (void);
		void clear(void){ free2 (_buf_org);}

		static int ARY_Load(int *f,char* fname,int d){
			FILE2 cmn;
			int num;
			cmn.open(fname,"r");
			num = cmn.ARY_Scan_INT(d);
			malloc2(f,(num)+1,EXIT0); // EXIT0どうにかする（-1にする？）
			cmn.reset();
			cmn.ARY_Read(f,num);
			cmn.close();
			return num;
		}
		
		static int ARY_Load(long long *f,char* fname,int d){
			FILE2 cmn;
			int num;
			cmn.open(fname,"r");
			num = cmn.ARY_Scan_INT(d);
			malloc2(f,(num)+1,EXIT0);
			cmn.reset();
			cmn.ARY_Read(f,num);
			cmn.close();
			return num;

		}

		static int ARY_Load(unsigned int *f,char* fname,int d){
			FILE2 cmn;
			int num;
			cmn.open(fname,"r");
			num = cmn.ARY_Scan_INT(d);
			malloc2(f,(num)+1,EXIT0); // EXIT0どうにかする（-1にする？）
			cmn.reset();
			cmn.ARY_Read(f,num);
			cmn.close();
			return num;
		}


		static int ARY_Load(double *f,char* fname,int d){
			FILE2 cmn;
			int num;
			cmn.open(fname,"r");
			num = cmn.ARY_Scan_DBL(d);
			malloc2(f,(num)+1,EXIT0);
			cmn.reset();
			cmn.ARY_Read(f,num);
			cmn.close();
			return num;
		}

		static void copy(char *f1, char *f2){
		  FILE *fp, *fp2;
			char buf[16384];
  		int s;

		  fopen2 (fp, f1, "r", EXIT);
		  fopen2 (fp2, f2, "w", EXIT);
			do {
				s = fread (buf, 1, 16384, fp);
				fwrite (buf, 1, s, fp2);
			} while (s);
		  fclose (fp);
  		fclose (fp2);
		}


	size_t ARY_Scan_INT(int d){
		size_t num=0;
		do{

			do{ read_int(); } while((FILE_err&((d)*5))==5);
	
			if(RANGE(5+(int)(d),FILE_err,6))break;
	
			(num)++;

		}while((FILE_err&(3-(int)(d)))==0);
	
		return num;
	}

	size_t ARY_Scan_DBL(int d){

		size_t num=0;

		do{

			do{ read_double(); } while((FILE_err&((d)*5))==5);
	
			if(RANGE(5+(int)(d),FILE_err,6))break;
	
			(num)++;

		}while((FILE_err&(3-(int)(d)))==0);
	
		return num;
	}
	void reset (){
 		 _buf = _buf_org;
 		 _buf_end = _buf_org-1;
 		 fseek (_fp, 0, SEEK_SET);
	}


	void ARY_Read(int *f,size_t num) {
	
		for (size_t i=0 ; i < num  ; i++){

 			do{
 				f[i]= read_int();
	 		}while((FILE_err&6)==4);

 			if(FILE_err&2)break;
	 	}
	}

	void ARY_Read(long long *f,size_t num) {
	
		for (size_t i=0 ; i < num  ; i++){

 			do{
 				f[i]= read_int();
	 		}while((FILE_err&6)==4);

 			if(FILE_err&2)break;
	 	}
	}

	void ARY_Read(unsigned int *f,size_t num) {
	
		for (size_t i=0 ; i < num  ; i++){

 			do{
 				f[i]= read_int();
	 		}while((FILE_err&6)==4);

 			if(FILE_err&2)break;
	 	}
	}

	void ARY_Read(double *f,size_t num) {
	
		for (size_t i=0 ; i < num  ; i++){
 			do{
 				f[i]=read_double();
 			}while((FILE_err&6)==4);

	 		if(FILE_err&2)break;
 		}
	}
	void close (){
  	fclose2 (_fp);
 		free2 (_buf_org);
	  _buf = _buf_end = 0;
	}

	FILE_LONG read_int ();
	double read_double ();
	WEIGHT read_WEIGHT ();

	size_t get_fsize(){
    fseek(_fp, 0, SEEK_END);
    size_t siz = ftell(_fp);
    fseek(_fp, 0, SEEK_SET);
    return siz;
  }

	size_t get_size(){
		return _buf-_buf_org;
  }
	bool exist_fp(){return _fp!=NULL;}
	int getc();

	void putc ( char c){
	  *(_buf) = c;
  	_buf++;
	}
	void puts ( char *s){
	  while ( *s != 0 ){
  	  *(_buf) = *s;
			s++;
    	_buf++;
	  }
	}
	void read_until_newline ();
	int read_pair ( LONG *x, LONG *y, WEIGHT *w, int flag);
	int read_item (FILE2 *wfp, LONG *x, LONG *y, WEIGHT *w, int fc, int flag);
	void closew();
	void print_int ( LONG n, char c);
	void print_real (double n, int len, char c);
	void print_WEIGHT ( WEIGHT w, int len, char c);
	void flush ();
	//FILE_COUNT count ( int flag, int skip_rows, int int_rows, int skip_clms, int int_clms, FILE_COUNT_INT row_limit);

};


