/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

/* AMCL mod p functions */
/* Small Finite Field arithmetic */
/* SU=m, SU is Stack Usage (NOT_SPECIAL Modulus) */

#include "fp_YYY.h"

using namespace XXX;

/* Fast Modular Reduction Methods */

/* r=d mod m */
/* d MUST be normalised */
/* Products must be less than pR in all cases !!! */
/* So when multiplying two numbers, their product *must* be less than MODBITS_YYY+BASEBITS_XXX*NLEN_XXX */
/* Results *may* be one bit bigger than MODBITS_YYY */

#if MODTYPE_YYY == PSEUDO_MERSENNE
/* r=d mod m */

/* Converts from BIG integer to residue form mod Modulus */
void YYY::FP_nres(FP *y,BIG x)
{
	BIG_copy(y->g,x);
	y->XES=1;
}

/* Converts from residue form back to BIG integer form */
void YYY::FP_redc(BIG x,FP *y)
{
	BIG_copy(x,y->g);
}

/* reduce a DBIG to a BIG exploiting the special form of the modulus */
void YYY::FP_mod(BIG r,DBIG d)
{
    BIG t,b;
    chunk v,tw;
    BIG_split(t,b,d,MODBITS_YYY);

    /* Note that all of the excess gets pushed into t. So if squaring a value with a 4-bit excess, this results in
       t getting all 8 bits of the excess product! So products must be less than pR which is Montgomery compatible */

    if (MConst < NEXCESS_XXX)
    {
        BIG_imul(t,t,MConst);
        BIG_norm(t);
		BIG_add(r,t,b);
		BIG_norm(r);
        tw=r[NLEN_XXX-1];
        r[NLEN_XXX-1]&=TMASK_YYY;
        r[0]+=MConst*((tw>>TBITS_YYY));
    }
    else
    {
        v=BIG_pmul(t,t,MConst);
		BIG_add(r,t,b);
		BIG_norm(r);
        tw=r[NLEN_XXX-1];
        r[NLEN_XXX-1]&=TMASK_YYY;
#if CHUNK == 16
        r[1]+=muladd(MConst,((tw>>TBITS_YYY)+(v<<(BASEBITS_XXX-TBITS_YYY))),0,&r[0]);
#else
        r[0]+=MConst*((tw>>TBITS_YYY)+(v<<(BASEBITS_XXX-TBITS_YYY)));
#endif
    }
    BIG_norm(r);
}
#endif

/* This only applies to Curve C448, so specialised (for now) */
#if MODTYPE_YYY == GENERALISED_MERSENNE

void YYY::FP_nres(FP *y,BIG x)
{
	BIG_copy(y->g,x);
	y->XES=1;
}

/* Converts from residue form back to BIG integer form */
void YYY::FP_redc(BIG x,FP *y)
{
	BIG_copy(x,y->g);
}

/* reduce a DBIG to a BIG exploiting the special form of the modulus */
void YYY::FP_mod(BIG r,DBIG d)
{
    BIG t,b;
    chunk carry;
    BIG_split(t,b,d,MBITS_YYY);

    BIG_add(r,t,b);

    BIG_dscopy(d,t);
    BIG_dshl(d,MBITS_YYY/2);

    BIG_split(t,b,d,MBITS_YYY);

    BIG_add(r,r,t);
    BIG_add(r,r,b);
    BIG_norm(r);
    BIG_shl(t,MBITS_YYY/2);

    BIG_add(r,r,t);

    carry=r[NLEN_XXX-1]>>TBITS_YYY;

    r[NLEN_XXX-1]&=TMASK_YYY;
    r[0]+=carry;

    r[224/BASEBITS_XXX]+=carry<<(224%BASEBITS_XXX); /* need to check that this falls mid-word */
    BIG_norm(r);
}

#endif

#if MODTYPE_YYY == MONTGOMERY_FRIENDLY

/* convert to Montgomery n-residue form */
void YYY::FP_nres(FP *y,BIG x)
{
    DBIG d;
    BIG r;
    BIG_rcopy(r,R2modp);
	BIG_mul(d,x,r);
	FP_mod(y->g,d);
	y->XES=2;
}

/* convert back to regular form */
void YYY::FP_redc(BIG x,FP *y)
{
    DBIG d;
    BIG_dzero(d);
    BIG_dscopy(d,y->g);
    FP_mod(x,d);
}

/* fast modular reduction from DBIG to BIG exploiting special form of the modulus */
void YYY::FP_mod(BIG a,DBIG d)
{
    int i;

    for (i=0; i<NLEN_XXX; i++)
        d[NLEN_XXX+i]+=muladd(d[i],MConst-1,d[i],&d[NLEN_XXX+i-1]);

    BIG_sducopy(a,d);
    BIG_norm(a);
}

#endif

#if MODTYPE_YYY == NOT_SPECIAL

/* convert to Montgomery n-residue form */
void YYY::FP_nres(FP *y,BIG x)
{
    DBIG d;
    BIG r;
    BIG_rcopy(r,R2modp);
	BIG_mul(d,x,r);
	FP_mod(y->g,d);
	y->XES=2;
}

/* convert back to regular form */
void YYY::FP_redc(BIG x,FP *y)
{
    DBIG d;
    BIG_dzero(d);
    BIG_dscopy(d,y->g);
    FP_mod(x,d);
}


/* reduce a DBIG to a BIG using Montgomery's no trial division method */
/* d is expected to be dnormed before entry */
/* SU= 112 */
void YYY::FP_mod(BIG a,DBIG d)
{
	BIG mdls;
	BIG_rcopy(mdls,Modulus);
	BIG_monty(a,mdls,MConst,d);
}

#endif

/* test x==0 ? */
/* SU= 48 */
int YYY::FP_iszilch(FP *x)
{
    BIG m,t;
    BIG_rcopy(m,Modulus);
	BIG_copy(t,x->g);
    BIG_mod(t,m);
    return BIG_iszilch(t);
}

void YYY::FP_copy(FP *y,FP *x)
{
	BIG_copy(y->g,x->g);
	y->XES=x->XES;
}

void YYY::FP_rcopy(FP *y, const BIG c)
{
	BIG b;
	BIG_rcopy(b,c);
	FP_nres(y,b);
}

/* Swap a and b if d=1 */
void YYY::FP_cswap(FP *a,FP *b,int d)
{
	sign32 t,c=d;
	BIG_cswap(a->g,b->g,d);

    c=~(c-1);
	t=c&((a->XES)^(b->XES));
	a->XES^=t;
	b->XES^=t;

}

/* Move b to a if d=1 */
void YYY::FP_cmove(FP *a,FP *b,int d)
{
	sign32 c=-d;

	BIG_cmove(a->g,b->g,d);
	a->XES^=(a->XES^b->XES)&c;
}

void YYY::FP_zero(FP *x)
{
	BIG_zero(x->g);
	x->XES=1;
}

int YYY::FP_equals(FP *x,FP *y)
{
//	int i;
//	chunk res=0;
	FP xg,yg;
	FP_copy(&xg,x);
	FP_copy(&yg,y);
	FP_reduce(&xg); FP_reduce(&yg);

//	for (i=0;i<NLEN_XXX;i++)
//	{
//		res |= xg.g[i]^yg.g[i];
//	}
//	return res;

	if (BIG_comp(xg.g,yg.g)==0) return 1;
	return 0;
}

/* output FP */
/* SU= 48 */
void YYY::FP_output(FP *r)
{
    BIG c;
    FP_redc(c,r);
    BIG_output(c);
}

void YYY::FP_rawoutput(FP *r)
{
    BIG_rawoutput(r->g);
}

#ifdef GET_STATS
int tsqr=0,rsqr=0,tmul=0,rmul=0;
int tadd=0,radd=0,tneg=0,rneg=0;
int tdadd=0,rdadd=0,tdneg=0,rdneg=0;
#endif

#ifdef FUSED_MODMUL

/* Insert fastest code here */

#endif

/* r=a*b mod Modulus */
/* product must be less that p.R - and we need to know this in advance! */
/* SU= 88 */
void YYY::FP_mul(FP *r,FP *a,FP *b)
{
    DBIG d;
    chunk ea,eb;
//    BIG_norm(a);
//    BIG_norm(b);
//    ea=EXCESS(a->g);
//    eb=EXCESS(b->g);


	if ((sign64)a->XES*b->XES>(sign64)FEXCESS_YYY)
	{
#ifdef DEBUG_REDUCE
        printf("Product too large - reducing it\n");
#endif
        FP_reduce(a);  /* it is sufficient to fully reduce just one of them < p */
	}

#ifdef FUSED_MODMUL
	FP_modmul(r->g,a->g,b->g);
#else
    BIG_mul(d,a->g,b->g);
    FP_mod(r->g,d);
#endif
	r->XES=2;
}


/* multiplication by an integer, r=a*c */
/* SU= 136 */
void YYY::FP_imul(FP *r,FP *a,int c)
{
    DBIG d;
	BIG k;
	FP f;
 
    int s=0;
   
    if (c<0)
    {
        c=-c;
        s=1;
    }
   
#if MODTYPE_YYY==PSEUDO_MERSENNE || MODTYPE_YYY==GENERALISED_MERSENNE
	
	BIG_pxmul(d,a->g,c);
	FP_mod(r->g,d);
	r->XES=2;

#else
	//Montgomery
	if (a->XES*c<=FEXCESS_YYY)
	{
		BIG_pmul(r->g,a->g,c);
		r->XES=a->XES*c;    // careful here - XES jumps!
	}
	else
	{// don't want to do this - only a problem for Montgomery modulus and larger constants
		BIG_zero(k);
		BIG_inc(k,c);
		BIG_norm(k);
		FP_nres(&f,k);
		FP_mul(r,a,&f);
	}
#endif
/*
    if (c<=NEXCESS_XXX && a->XES*c <= FEXCESS_YYY)
	{
        BIG_imul(r->g,a->g,c);
		r->XES=a->XES*c;
		FP_norm(r);
	}
    else
    {
            BIG_pxmul(d,a->g,c);

            BIG_rcopy(m,Modulus);
			BIG_dmod(r->g,d,m);
            //FP_mod(r->g,d);                /// BIG problem here! Too slow for PM, How to do fast for Monty?
			r->XES=2;
    }
*/
    if (s) 
	{
		FP_neg(r,r);
		FP_norm(r);
	}
}

/* Set r=a^2 mod m */
/* SU= 88 */
void YYY::FP_sqr(FP *r,FP *a)
{
    DBIG d;
//    chunk ea;
//    BIG_norm(a);
//    ea=EXCESS(a->g);


	if ((sign64)a->XES*a->XES>(sign64)FEXCESS_YYY)
	{
#ifdef DEBUG_REDUCE
        printf("Product too large - reducing it\n");
#endif
        FP_reduce(a);
    }

    BIG_sqr(d,a->g);
    FP_mod(r->g,d);
	r->XES=2;
}

/* SU= 16 */
/* Set r=a+b */
void YYY::FP_add(FP *r,FP *a,FP *b)
{
    BIG_add(r->g,a->g,b->g);
	r->XES=a->XES+b->XES;
	if (r->XES>FEXCESS_YYY)
	{
#ifdef DEBUG_REDUCE
        printf("Sum too large - reducing it \n");
#endif
        FP_reduce(r);
    }
}

/* Set r=a-b mod m */
/* SU= 56 */
void YYY::FP_sub(FP *r,FP *a,FP *b)
{
    FP n;
//	BIG_norm(b);
    FP_neg(&n,b);
//	BIG_norm(n);
    FP_add(r,a,&n);
}

// https://graphics.stanford.edu/~seander/bithacks.html
// constant time log to base 2 (or number of bits in)

static int logb2(unsign32 v)
{
    int r;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    r = (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
    return r;
}

// find appoximation to quotient of a/m
// Out by at most 2.
// Note that MAXXES is bounded to be 2-bits less than half a word
static int quo(BIG n,BIG m)
{
	int sh;
	chunk num,den;
	int hb=CHUNK/2;
	if (TBITS_YYY<hb)
	{
		sh=hb-TBITS_YYY;
		num=(n[NLEN_XXX-1]<<sh)|(n[NLEN_XXX-2]>>(BASEBITS_XXX-sh));
		den=(m[NLEN_XXX-1]<<sh)|(m[NLEN_XXX-2]>>(BASEBITS_XXX-sh));
	}
	else
	{
		num=n[NLEN_XXX-1];
		den=m[NLEN_XXX-1];
	}
	return (int)(num/(den+1));
}

/* SU= 48 */
/* Fully reduce a mod Modulus */
void YYY::FP_reduce(FP *a)
{
    BIG m,r;
	int sr,sb,q;
	chunk carry;

    BIG_rcopy(m,Modulus);
	BIG_norm(a->g);

	if (a->XES>16)
	{
		q=quo(a->g,m);
		carry=BIG_pmul(r,m,q);
		r[NLEN_XXX-1]+=(carry<<BASEBITS_XXX); // correction - put any carry out back in again
		BIG_sub(a->g,a->g,r);
		BIG_norm(a->g);
		sb=2;
	}
	else sb=logb2(a->XES-1);  // sb does not depend on the actual data

	BIG_fshl(m,sb);
	while (sb>0)
	{
// constant time...
		sr=BIG_ssn(r,a->g,m);  // optimized combined shift, subtract and norm
		BIG_cmove(a->g,r,1-sr);
		sb--;
	}
/*
    BIG_rcopy(m,Modulus);
	if (BIG_comp(a->g,m)>0)
	{
		printf("NOT fully reduced q=%x %x %x %x\n",q,quo(a->g,m),FEXCESS_YYY,a->XES);
		exit(0);
	}
*/
	a->XES=1;
}

void YYY::FP_norm(FP *x)
{
	BIG_norm(x->g);
}

/* Set r=-a mod Modulus */
/* SU= 64 */
void YYY::FP_neg(FP *r,FP *a)
{
    int sb;
    BIG m;

    BIG_rcopy(m,Modulus);

    sb=logb2(a->XES-1);
    BIG_fshl(m,sb);
    BIG_sub(r->g,m,a->g);
	r->XES=((sign32)1<<sb)+1;  // +1 to cover case where a is zero ?

    if (r->XES>FEXCESS_YYY)
    {
#ifdef DEBUG_REDUCE
        printf("Negation too large -  reducing it \n");
#endif
        FP_reduce(r);
	}

}

/* Set r=a/2. */
/* SU= 56 */
void YYY::FP_div2(FP *r,FP *a)
{
    BIG m;
    BIG_rcopy(m,Modulus);
    FP_copy(r,a);
//    BIG_norm(a);
    if (BIG_parity(a->g)==0)
    {

        BIG_fshr(r->g,1);
    }
    else
    {
        BIG_add(r->g,r->g,m);
        BIG_norm(r->g);
        BIG_fshr(r->g,1);
    }
}

/* set w=1/x */
void YYY::FP_inv(FP *w,FP *x)
{
 
	BIG m2;
	BIG_rcopy(m2,Modulus);
	BIG_dec(m2,2);
	BIG_norm(m2);
	FP_pow(w,x,m2);

/*
    BIG m,b;
    BIG_rcopy(m,Modulus);
    FP_redc(b,x);
    BIG_invmodp(b,b,m);
    FP_nres(w,b);
*/
}

/* SU=8 */
/* set n=1 */
void YYY::FP_one(FP *n)
{
	BIG b;
    BIG_one(b);
    FP_nres(n,b);
}

/* Set r=a^b mod Modulus */
/* SU= 136 */
/*
void YYY::FP_pow(FP *r,FP *a,BIG b)
{
    BIG z,zilch;
	FP w;
    int bt;
    BIG_zero(zilch);

	FP_norm(a);
    BIG_norm(b);
    BIG_copy(z,b);
    FP_copy(&w,a);
    FP_one(r);
    while(1)
    {
        bt=BIG_parity(z);
        BIG_fshr(z,1);
        if (bt) FP_mul(r,r,&w);
        if (BIG_comp(z,zilch)==0) break;
        FP_sqr(&w,&w);
    }
    FP_reduce(r);
}
*/

void YYY::FP_pow(FP *r,FP *a,BIG b)
{
	sign8 w[1+(NLEN_XXX*BASEBITS_XXX+3)/4];
	FP tb[16];
	BIG t;
	int i,nb;

	FP_norm(a);
    BIG_norm(b);
	BIG_copy(t,b);
	nb=1+(BIG_nbits(t)+3)/4;
    // convert exponent to 4-bit window 
    for (i=0; i<nb; i++)
    {
        w[i]=BIG_lastbits(t,4);
        BIG_dec(t,w[i]);
        BIG_norm(t);
        BIG_fshr(t,4);
    }	

	FP_one(&tb[0]);
	FP_copy(&tb[1],a);
	for (i=2;i<16;i++)
		FP_mul(&tb[i],&tb[i-1],a);
	
	FP_copy(r,&tb[w[nb-1]]);
    for (i=nb-2; i>=0; i--)
    {
		FP_sqr(r,r);
		FP_sqr(r,r);
		FP_sqr(r,r);
		FP_sqr(r,r);
		FP_mul(r,r,&tb[w[i]]);
	}
    FP_reduce(r);
}

/* is r a QR? */
int YYY::FP_qr(FP *r)
{
    int j;
    BIG m;
	BIG b;
    BIG_rcopy(m,Modulus);
    FP_redc(b,r);
    j=BIG_jacobi(b,m);
    FP_nres(r,b);
    if (j==1) return 1;
    return 0;

}

/* Set a=sqrt(b) mod Modulus */
/* SU= 160 */
void YYY::FP_sqrt(FP *r,FP *a)
{
	FP v,i;
    BIG b;
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_mod(a->g,m);
    BIG_copy(b,m);
    if (MOD8_YYY==5)
    {
        BIG_dec(b,5);
        BIG_norm(b);
        BIG_fshr(b,3); /* (p-5)/8 */
        FP_copy(&i,a);
        BIG_fshl(i.g,1);
        FP_pow(&v,&i,b);
        FP_mul(&i,&i,&v);
        FP_mul(&i,&i,&v);
        BIG_dec(i.g,1);
        FP_mul(r,a,&v);
        FP_mul(r,r,&i);
        FP_reduce(r);
    }
    if (MOD8_YYY==3 || MOD8_YYY==7)
    {
        BIG_inc(b,1);
        BIG_norm(b);
        BIG_fshr(b,2); /* (p+1)/4 */
        FP_pow(r,a,b);
    }
}

/*
int main()
{

	BIG r;

	FP_one(r);
	FP_sqr(r,r);

	BIG_output(r);

	int i,carry;
	DBIG c={0,0,0,0,0,0,0,0};
	BIG a={1,2,3,4};
	BIG b={3,4,5,6};
	BIG r={11,12,13,14};
	BIG s={23,24,25,15};
	BIG w;

//	printf("NEXCESS_XXX= %d\n",NEXCESS_XXX);
//	printf("MConst= %d\n",MConst);

	BIG_copy(b,Modulus);
	BIG_dec(b,1);
	BIG_norm(b);

	BIG_randomnum(r); BIG_norm(r); BIG_mod(r,Modulus);
//	BIG_randomnum(s); norm(s); BIG_mod(s,Modulus);

//	BIG_output(r);
//	BIG_output(s);

	BIG_output(r);
	FP_nres(r);
	BIG_output(r);
	BIG_copy(a,r);
	FP_redc(r);
	BIG_output(r);
	BIG_dscopy(c,a);
	FP_mod(r,c);
	BIG_output(r);


//	exit(0);

//	copy(r,a);
	printf("r=   "); BIG_output(r);
	BIG_modsqr(r,r,Modulus);
	printf("r^2= "); BIG_output(r);

	FP_nres(r);
	FP_sqrt(r,r);
	FP_redc(r);
	printf("r=   "); BIG_output(r);
	BIG_modsqr(r,r,Modulus);
	printf("r^2= "); BIG_output(r);


//	for (i=0;i<100000;i++) FP_sqr(r,r);
//	for (i=0;i<100000;i++)
		FP_sqrt(r,r);

	BIG_output(r);
}
*/
