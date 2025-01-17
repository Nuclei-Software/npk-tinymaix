/* Copyright 2022 Sipeed Technology Co., Ltd. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "stdlib.h"
#include "stdint.h"
#include "math.h"
#include "tm_port.h"

#ifndef __riscv_vector
#error "Vector intrinsics require the vector extension."
#endif
#include <riscv_vector.h>

//RV64 V-extend acceleration
//https://github.com/T-head-Semi/openc906/tree/main/doc
//https://github.com/riscv-non-isa/rvv-intrinsic-doc
//https://occ.t-head.cn/community/download?id=4073475960903634944
//https://github.com/riscv/riscv-v-spec
//Use this toolchain support rvv inntrinsic: Xuantie-900-gcc-elf-newlib-x86_64-V2.2.4-20211227.tar.gz
//-march=rv64gcv0p7_zfh_xtheadc 
//more information refer to: 
//https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource//1659318003104/Xuantie+900+Series+RVV-0.7.1+Intrinsic+Manual.pdf
//https://riscv.org/wp-content/uploads/2019/06/17.40-Vector_RISCV-20190611-Vectors.pdf
//Total 32 vector registers
//ELEN: Size of largest element in bits
//VLEN: Number of bits in each vector register; VLEN>=ELEN
//SLEN: Striping distance in bits; VLEN >= SLEN >= ELEN
//SEW:  Standard element width, =8,16,32,...,1024
//LMUL: Vector length multiplier, =1,2,4,8  (LMUL registers at the same time)
//EDIV: Vector element divider, =1,2,4,8
//vtype: Vtype sets width of element in each vector register (e.g., 16-bit, 32-bit, …)
//vl: Vector length CSR sets number of elements active in each instruction
//vstart: Resumption element after trap
//fcsr(vxrm/vxsat): Fixed-point rounding mode and saturation flag fields in FP CSR


#define RVV_VLEN 128  //TODO: read csrr_vlenb
#define ENABLE_NUCLEI_EXT 1

/********************************** TM_MDL_FP32 ***********************************************/
#if TM_MDL_TYPE==TM_MDL_FP32
#define PACK_N (RVV_VLEN/32)    //fp32
uint32_t tdot = 0;
#ifndef ENABLE_NUCLEI_EXT
TM_INLINE  void tm_dot_prod(mtype_t* sptr, mtype_t* kptr,uint32_t size, sumtype_t* result)
{
    float sumbuf[PACK_N];
    float sum = 0.f;
    int cnt=size/PACK_N;
    //uint64_t t0,t1;
    //__ASM volatile("csrr %0, mcycle" : "=r"(t0));
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e32m1(PACK_N);
        vfloat32m1_t sumv = __riscv_vfmv_v_f_f32m1(0.f, vl);   //set sum=0
        vfloat32m1_t v_zero = __riscv_vfmv_v_f_f32m1(0.f, vl);
        for(int i=0; i<cnt; i++){
            vfloat32m1_t s = __riscv_vle32_v_f32m1(sptr, vl);
            vfloat32m1_t k = __riscv_vle32_v_f32m1(kptr, vl);
            sumv = __riscv_vfmacc_vv_f32m1(sumv, s, k, vl);
            sptr += PACK_N;
            kptr += PACK_N;
        }
        v_zero = __riscv_vfredosum_vs_f32m1_f32m1(sumv, v_zero, vl);
        sum = __riscv_vfmv_f_s_f32m1_f32(v_zero);
    }
    for(int i=0; i<size%PACK_N; i++){
        sum += *sptr * *kptr;
        sptr++;kptr++;
    }

    *result = sum;
    //__ASM volatile("csrr %0, mcycle" : "=r"(t1));
    //printf("size=%3d, dt=%ld\r\n", size, t1-t0);
    //tdot+=(t1-t0);
    return;
}

TM_INLINE  void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{ 
    float sumbuf[PACK_N];
    mtype_t* kptr0 = kptr;
    mtype_t* kptr1 = kptr+size;
    //mtype_t* kptr2 = kptr+size*2;
    //mtype_t* kptr3 = kptr+size*3;
    float sum0 = 0;
    float sum1 = 0;
    //float sum2 = 0;
    //float sum3 = 0;
    int cnt=size/PACK_N;
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e32m1(PACK_N);
        vfloat32m1_t sumv0 = __riscv_vfmv_v_f_f32m1(0.f, vl);   //set sum=0
        vfloat32m1_t sumv1 = __riscv_vfmv_v_f_f32m1(0.f, vl);   //set sum=0
        //vfloat32m1_t sumv2 = __riscv_vfmv_v_f_f32m1(0.f, vl);   //set sum=0
        //vfloat32m1_t sumv3 = __riscv_vfmv_v_f_f32m1(0.f, vl);   //set sum=0
        vfloat32m1_t v_zero0 = __riscv_vfmv_v_f_f32m1(0.0f, vl);
        vfloat32m1_t v_zero1 = __riscv_vfmv_v_f_f32m1(0.0f, vl);
        //vfloat32m1_t v_zero2 = __riscv_vfmv_v_f_f32m1(0.0f, vl);
        //vfloat32m1_t v_zero3 = __riscv_vfmv_v_f_f32m1(0.0f, vl);
        for(int i=0; i<cnt; i++){
            vfloat32m1_t s  = __riscv_vle32_v_f32m1(sptr, vl);
            vfloat32m1_t k0 = __riscv_vle32_v_f32m1(kptr0, vl);
            vfloat32m1_t k1 = __riscv_vle32_v_f32m1(kptr1, vl);
            //vfloat32m1_t k2 = __riscv_vle32_v_f32m1(kptr2, vl);
            //vfloat32m1_t k3 = __riscv_vle32_v_f32m1(kptr3, vl);
            sumv0 = __riscv_vfmacc_vv_f32m1(sumv0, s, k0, vl);
            sumv1 = __riscv_vfmacc_vv_f32m1(sumv1, s, k1, vl);
            //sumv2 = __riscv_vfmacc_vv_f32m1(sumv2, s, k2, vl);
            //sumv3 = __riscv_vfmacc_vv_f32m1(sumv3, s, k3, vl);
            sptr  += PACK_N;
            kptr0 += PACK_N;
            kptr1 += PACK_N;
            //kptr2 += PACK_N;
            //kptr3 += PACK_N;
        }
        v_zero0 = __riscv_vfredosum_vs_f32m1_f32m1(sumv0, v_zero0, vl);
        v_zero1 = __riscv_vfredosum_vs_f32m1_f32m1(sumv1, v_zero1, vl);
        //v_zero2 = __riscv_vfredosum_vs_f32m1_f32m1(sumv2, v_zero2, vl);
        //v_zero3 = __riscv_vfredosum_vs_f32m1_f32m1(sumv3, v_zero3, vl);
        sum0 = __riscv_vfmv_f_s_f32m1_f32(v_zero0);
        sum1 = __riscv_vfmv_f_s_f32m1_f32(v_zero1);
        //sum2 = __riscv_vfmv_f_s_f32m1_f32(v_zero2);
        //sum3 = __riscv_vfmv_f_s_f32m1_f32(v_zero3);
    }
    for(int i=0; i<size%PACK_N; i++){
        sum0 += *sptr * *kptr0;
        sum1 += *sptr * *kptr1;
        //sum2 += *sptr * *kptr2;
        //sum3 += *sptr * *kptr3;
        sptr++;kptr0++;kptr1++;//kptr2++;kptr3++;
    }

    result[0] = sum0;
    result[1] = sum1;
    //result[2] = sum2;
    //result[3] = sum3;
    return;
}
#else

#define PACK_N2 (RVV_VLEN/32*8)    //fp32
#define PACK_N3 (RVV_VLEN/32*4)    //fp32
TM_INLINE  void tm_dot_prod(mtype_t* sptr, mtype_t* kptr,uint32_t size, sumtype_t* result)
{
    float sum = 0.f;
    int cnt=size/PACK_N2;
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e32m8(PACK_N2);
        vfloat32m8_t sumv = __riscv_vfmv_v_f_f32m8(0.f, vl);   //set sum=0
        vfloat32m1_t v_zero = __riscv_vfmv_v_f_f32m1(0.f, vl);
        for(int i=0; i<cnt; i++){
            vfloat32m8_t s = __riscv_vle32_v_f32m8(sptr, vl);
            sptr += PACK_N2;
            vfloat32m8_t k = __riscv_vle32_v_f32m8(kptr, vl);
            kptr += PACK_N2;
            sumv = __riscv_vfmacc_vv_f32m8(sumv, s, k, vl);
        }
        v_zero = __riscv_vfredosum_vs_f32m8_f32m1(sumv, v_zero, vl);
        sum = __riscv_vfmv_f_s_f32m1_f32(v_zero);
    }
    for(int i=0; i<size%PACK_N2; i++){
        sum += *sptr * *kptr;
        sptr++;kptr++;
    }

    *result = sum;
    return;
}

TM_INLINE  void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{
    mtype_t* kptr0 = kptr;
    mtype_t* kptr1 = kptr+size;
    float sum0 = 0;
    float sum1 = 0;
    int cnt=size/PACK_N3;
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e32m4(PACK_N3);
        vfloat32m4_t sumv0 = __riscv_vfmv_v_f_f32m4(0.f, vl);   //set sum=0
        vfloat32m4_t sumv1 = __riscv_vfmv_v_f_f32m4(0.f, vl);   //set sum=0
        vfloat32m1_t v_zero0 = __riscv_vfmv_v_f_f32m1(0.0f, vl);
        vfloat32m1_t v_zero1 = __riscv_vfmv_v_f_f32m1(0.0f, vl);
        for(int i=0; i<cnt; i++){
            vfloat32m4_t s  = __riscv_vle32_v_f32m4(sptr, vl);
            sptr += PACK_N3;
            vfloat32m4_t k0 = __riscv_vle32_v_f32m4(kptr0, vl);
            kptr0 += PACK_N3;
            vfloat32m4_t k1 = __riscv_vle32_v_f32m4(kptr1, vl);
            kptr1 += PACK_N3;
            sumv0 = __riscv_vfmacc_vv_f32m4(sumv0, s, k0, vl);
            sumv1 = __riscv_vfmacc_vv_f32m4(sumv1, s, k1, vl);
        }
        v_zero0 = __riscv_vfredosum_vs_f32m4_f32m1(sumv0, v_zero0, vl);
        v_zero1 = __riscv_vfredosum_vs_f32m4_f32m1(sumv1, v_zero1, vl);
        sum0 = __riscv_vfmv_f_s_f32m1_f32(v_zero0);
        sum1 = __riscv_vfmv_f_s_f32m1_f32(v_zero1);
    }
    for(int i=0; i<size%PACK_N3; i++){
        sum0 += *sptr * *kptr0;
        sum1 += *sptr * *kptr1;
        sptr++;kptr0++;kptr1++;
    }

    result[0] = sum0;
    result[1] = sum1;
    return;
}
#endif /* #if ENABLE_NUCLEI_EXT */

TM_INLINE void tm_dot_prod_gap_3x3x1(mtype_t* sptr, mtype_t* kptr, uint32_t* k_oft, sumtype_t* result)
{
    *result = sptr[k_oft[0]]*kptr[0] + sptr[k_oft[1]]*kptr[1] + sptr[k_oft[2]]*kptr[2] + \
        sptr[k_oft[3]]*kptr[3] + sptr[k_oft[4]]*kptr[4] + sptr[k_oft[5]]*kptr[5] + \
        sptr[k_oft[6]]*kptr[6] + sptr[k_oft[7]]*kptr[7] + sptr[k_oft[8]]*kptr[8] ;
    return;                  
}

TM_INLINE void tm_dot_prod_3x3x1(mtype_t* sptr, mtype_t* kptr, sumtype_t* result)
{
    *result = sptr[0]*kptr[0] + sptr[1]*kptr[1] + sptr[2]*kptr[2] + \
        sptr[3]*kptr[3] + sptr[4]*kptr[4] + sptr[5]*kptr[5] + \
        sptr[6]*kptr[6] + sptr[7]*kptr[7] + sptr[8]*kptr[8] ;
    return;
}

/********************************** TM_MDL_FP16 ***********************************************/
#elif TM_MDL_TYPE==TM_MDL_FP16
#define PACK_N (RVV_VLEN/16)    //fp16, packn=8
TM_INLINE  void tm_dot_prod(mtype_t* sptr, mtype_t* kptr,uint32_t size, sumtype_t* result)
{ 
    float16_t sumbuf[PACK_N];
    float16_t sum = 0;
    int cnt=size/PACK_N;
    //uint64_t t0,t1;
    //__ASM volatile("csrr %0, mcycle" : "=r"(t0));
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e16m1(PACK_N);
        vfloat16m1_t sumv = __riscv_vfmv_v_f_f16m1(0.f, vl);   //set sum=0
        vfloat16m1_t v_zero = __riscv_vfmv_v_f_f16m1(0.0f, vl);
        for(int i=0; i<cnt; i++){
            vfloat16m1_t s = __riscv_vle16_v_f16m1(sptr, vl);
            vfloat16m1_t k = __riscv_vle16_v_f16m1(kptr, vl);
            sumv = __riscv_vfmacc_vv_f16m1(sumv, s, k, vl);
            sptr += PACK_N;
            kptr += PACK_N;
        }
        v_zero = __riscv_vfredosum_vs_f16m1_f16m1(sumv, v_zero, vl);
        sum = __riscv_vfmv_f_s_f16m1_f16(v_zero);
    }
    for(int i=0; i<size%PACK_N; i++){
        sum += *sptr * *kptr;
        sptr++;kptr++;
    }

    *result = sum;
    return;
}

TM_INLINE  void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{ 
    float16_t sumbuf[PACK_N];
    mtype_t* kptr0 = kptr;
    mtype_t* kptr1 = kptr+size;
    //mtype_t* kptr2 = kptr+size*2;
    //mtype_t* kptr3 = kptr+size*3;
    float16_t sum0 = 0;
    float16_t sum1 = 0;
    //float16_t sum2 = 0;
    //float16_t sum3 = 0;
    int cnt=size/PACK_N;
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e16m1(PACK_N);
        vfloat16m1_t sumv0 = __riscv_vfmv_v_f_f16m1(0.f, vl);   //set sum=0
        vfloat16m1_t sumv1 = __riscv_vfmv_v_f_f16m1(0.f, vl);   //set sum=0
        //vfloat16m1_t sumv2 = __riscv_vfmv_v_f_f16m1(0.f, vl);   //set sum=0
        //vfloat16m1_t sumv3 = __riscv_vfmv_v_f_f16m1(0.f, vl);   //set sum=0
        vfloat16m1_t v_zero0 = __riscv_vfmv_v_f_f16m1(0.0f, vl);
        vfloat16m1_t v_zero1 = __riscv_vfmv_v_f_f16m1(0.0f, vl);
        //vfloat16m1_t v_zero2 = __riscv_vfmv_v_f_f16m1(0.0f, vl);
        //vfloat16m1_t v_zero3 = __riscv_vfmv_v_f_f16m1(0.0f, vl);
        for(int i=0; i<cnt; i++){
            vfloat16m1_t s = __riscv_vle16_v_f16m1(sptr, vl);
            vfloat16m1_t k0 = __riscv_vle16_v_f16m1(kptr0, vl);
            vfloat16m1_t k1 = __riscv_vle16_v_f16m1(kptr1, vl);
            //vfloat16m1_t k2 = __riscv_vle16_v_f16m1(kptr2, vl);
            //vfloat16m1_t k3 = __riscv_vle16_v_f16m1(kptr3, vl);
            sumv0 = __riscv_vfmacc_vv_f16m1(sumv0, s, k0, vl);
            sumv1 = __riscv_vfmacc_vv_f16m1(sumv1, s, k1, vl);
            //sumv2 = __riscv_vfmacc_vv_f16m1(sumv2, s, k2, vl);
            //sumv3 = __riscv_vfmacc_vv_f16m1(sumv3, s, k3, vl);
            sptr += PACK_N;
            kptr0 += PACK_N;
            kptr1 += PACK_N;
            //kptr2 += PACK_N;
            //kptr3 += PACK_N;
        }
        v_zero0 = __riscv_vfredosum_vs_f16m1_f16m1(sumv0, v_zero0, vl);
        v_zero1 = __riscv_vfredosum_vs_f16m1_f16m1(sumv1, v_zero1, vl);
        //v_zero2 = __riscv_vfredosum_vs_f16m1_f16m1(sumv2, v_zero2, vl);
        //v_zero3 = __riscv_vfredosum_vs_f16m1_f16m1(sumv3, v_zero3, vl);
        sum0 = __riscv_vfmv_f_s_f16m1_f16(v_zero0);
        sum1 = __riscv_vfmv_f_s_f16m1_f16(v_zero1);
        //sum2 = __riscv_vfmv_f_s_f16m1_f16(v_zero2);
        //sum3 = __riscv_vfmv_f_s_f16m1_f16(v_zero3);
    }
    for(int i=0; i<size%PACK_N; i++){
        sum0 += *sptr * *kptr0;
        sum1 += *sptr * *kptr1;
        //sum2 += *sptr * *kptr2;
        //sum3 += *sptr * *kptr3;
        sptr++;kptr0++;kptr1++;//kptr2++;kptr3++;
    }

    result[0] = sum0;
    result[1] = sum1;
    //result[2] = sum2;
    //result[3] = sum3;
    return;
}

TM_INLINE void tm_dot_prod_gap_3x3x1(mtype_t* sptr, mtype_t* kptr, uint32_t* k_oft, sumtype_t* result)
{
    *result = sptr[k_oft[0]]*kptr[0] + sptr[k_oft[1]]*kptr[1] + sptr[k_oft[2]]*kptr[2] + \
        sptr[k_oft[3]]*kptr[3] + sptr[k_oft[4]]*kptr[4] + sptr[k_oft[5]]*kptr[5] + \
        sptr[k_oft[6]]*kptr[6] + sptr[k_oft[7]]*kptr[7] + sptr[k_oft[8]]*kptr[8] ;
    return;
}

TM_INLINE void tm_dot_prod_3x3x1(mtype_t* sptr, mtype_t* kptr, sumtype_t* result)
{
    *result = sptr[0]*kptr[0] + sptr[1]*kptr[1] + sptr[2]*kptr[2] + \
        sptr[3]*kptr[3] + sptr[4]*kptr[4] + sptr[5]*kptr[5] + \
        sptr[6]*kptr[6] + sptr[7]*kptr[7] + sptr[8]*kptr[8] ;
    return;
}

/********************************** TM_MDL_INT8 ***********************************************/
#elif TM_MDL_TYPE==TM_MDL_INT8
#define PACK_N (RVV_VLEN/8)    //int8  16
//i8 to i16 (Widening Floating-Point/Integer Type-Convert Operations)
//  vint16m2_t __riscv_vwcvt_x_x_v_i16m2 (vint8m1_t src, size_t vl);
//Vector Widening Integer Multiply-Add Operations
//  vint32m4_t __riscv_vwmacc_vv_i32m4 (vint32m4_t vd, vint16m2_t vs1, vint16m2_t vs2, size_t vl);
uint32_t tdot = 0;
TM_STATIC uint32_t size0=0;
#ifndef ENABLE_NUCLEI_EXT
TM_INLINE  void tm_dot_prod(mtype_t* sptr, mtype_t* kptr,uint32_t size, sumtype_t* result)
{ 
    int32_t sumbuf[PACK_N];
    int32_t sum = 0;
    int cnt=size/PACK_N;
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e8m1(PACK_N);
        vint32m4_t sumv = __riscv_vmv_v_x_i32m4(0, vl);         //set sum=0
        vint32m1_t v_zero = __riscv_vmv_v_x_i32m1(0, vl);
        for(int i=0; i<cnt; i++){
            vint8m1_t  s8  = __riscv_vle8_v_i8m1(sptr, vl);     //load i8
            vint8m1_t  k8  = __riscv_vle8_v_i8m1(kptr, vl);
            vint16m2_t s16 = __riscv_vwcvt_x_x_v_i16m2(s8,vl);  //cvt i8 to i16
            vint16m2_t k16 = __riscv_vwcvt_x_x_v_i16m2(k8,vl);
            sumv = __riscv_vwmacc_vv_i32m4(sumv, s16, k16, vl); //mac
            sptr += PACK_N;
            kptr += PACK_N;
        }
        v_zero = __riscv_vredsum_vs_i32m4_i32m1(sumv, v_zero, vl);
        sum = __riscv_vmv_x_s_i32m1_i32(v_zero);
    }
    for(int i=0; i<size%PACK_N; i++){
        sum += *sptr * *kptr;
        sptr++;kptr++;
    }

    *result = sum;
    return;
}

TM_INLINE  void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{ 
    int32_t sum0 = 0;
    int32_t sum1 = 0;
    mtype_t* kptr0 = kptr;
    mtype_t* kptr1 = kptr+size;
    int cnt=size/PACK_N;
    if(cnt>0){
        size_t vl = __riscv_vsetvl_e8m1(PACK_N);
        vint32m4_t sumv0 = __riscv_vmv_v_x_i32m4(0, vl);         //set sum=0
        vint32m4_t sumv1 = __riscv_vmv_v_x_i32m4(0, vl);         //set sum=0
        vint32m1_t v_zero0 = __riscv_vmv_v_x_i32m1(0, vl);
        vint32m1_t v_zero1 = __riscv_vmv_v_x_i32m1(0, vl);
        for(int i=0; i<cnt; i++){
            vint8m1_t  s8  = __riscv_vle8_v_i8m1(sptr, vl);     //load i8
            vint8m1_t  k80  = __riscv_vle8_v_i8m1(kptr0, vl);
            vint8m1_t  k81  = __riscv_vle8_v_i8m1(kptr1, vl);
            vint16m2_t s16 = __riscv_vwcvt_x_x_v_i16m2(s8,vl);  //cvt i8 to i16
            vint16m2_t k160 = __riscv_vwcvt_x_x_v_i16m2(k80,vl);
            vint16m2_t k161 = __riscv_vwcvt_x_x_v_i16m2(k81,vl);
            sumv0 = __riscv_vwmacc_vv_i32m4(sumv0, s16, k160, vl); //mac
            sumv1 = __riscv_vwmacc_vv_i32m4(sumv1, s16, k161, vl); //mac
            sptr += PACK_N;
            kptr0 += PACK_N;
            kptr1 += PACK_N;
        }
        v_zero0 = __riscv_vredsum_vs_i32m4_i32m1(sumv0, v_zero0, vl);
        v_zero1 = __riscv_vredsum_vs_i32m4_i32m1(sumv1, v_zero1, vl);
        sum0 = __riscv_vmv_x_s_i32m1_i32(v_zero0);
        sum1 = __riscv_vmv_x_s_i32m1_i32(v_zero1);
    }
    for(int i=0; i<size%PACK_N; i++){
        sum0 += *sptr * *kptr0;
        sum1 += *sptr * *kptr1;
        sptr++;kptr0++;kptr1++;
    }

    result[0] = sum0;
    result[1] = sum1;
    return;
}

TM_INLINE void tm_dot_prod_gap_3x3x1(mtype_t* sptr, mtype_t* kptr, uint32_t* k_oft, sumtype_t* result)
{
    *result = sptr[k_oft[0]]*kptr[0] + sptr[k_oft[1]]*kptr[1] + sptr[k_oft[2]]*kptr[2] + \
        sptr[k_oft[3]]*kptr[3] + sptr[k_oft[4]]*kptr[4] + sptr[k_oft[5]]*kptr[5] + \
        sptr[k_oft[6]]*kptr[6] + sptr[k_oft[7]]*kptr[7] + sptr[k_oft[8]]*kptr[8] ;
    return;
}

TM_INLINE void tm_dot_prod_3x3x1(mtype_t* sptr, mtype_t* kptr, sumtype_t* result)
{
    *result = sptr[0]*kptr[0] + sptr[1]*kptr[1] + sptr[2]*kptr[2] + \
        sptr[3]*kptr[3] + sptr[4]*kptr[4] + sptr[5]*kptr[5] + \
        sptr[6]*kptr[6] + sptr[7]*kptr[7] + sptr[8]*kptr[8] ;
    return;
}
#else

TM_INLINE  void tm_dot_prod(mtype_t* sptr, mtype_t* kptr,uint32_t size, sumtype_t* result)
{
    int32_t sum = 0;
    size_t vl;
    int32_t blkCnt = size;
    if (size < 32) {
        vl = 1;
        vint8m1_t s8, k8;
        vint32m1_t v_sum = __riscv_vmv_v_x_i32m1(0, vl);
        for (; (vl = __riscv_vsetvl_e8m1(blkCnt)) > 0; blkCnt -= vl) {
            s8  = __riscv_vle8_v_i8m1(sptr, vl);
            sptr += vl;
            k8  = __riscv_vle8_v_i8m1(kptr, vl);
            kptr += vl;
            v_sum = __riscv_vwredsum_vs_i16m2_i32m1(__riscv_vwmul_vv_i16m2(s8, k8, vl), v_sum, vl);
        }
        sum = __riscv_vmv_x_s_i32m1_i32(v_sum);
    } else {
        vl = 1;
        vint8m4_t s8, k8;
        vint32m1_t v_sum = __riscv_vmv_v_x_i32m1(0, vl);
        for (; (vl = __riscv_vsetvl_e8m4(blkCnt)) > 0; blkCnt -= vl) {
            s8  = __riscv_vle8_v_i8m4(sptr, vl);
            sptr += vl;
            k8  = __riscv_vle8_v_i8m4(kptr, vl);
            kptr += vl;
            v_sum = __riscv_vwredsum_vs_i16m8_i32m1(__riscv_vwmul_vv_i16m8(s8, k8, vl), v_sum, vl);
        }
        sum = __riscv_vmv_x_s_i32m1_i32(v_sum);
    }

    *result = sum;
    return;
}

TM_INLINE  void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{
    int32_t sum0 = 0;
    int32_t sum1 = 0;
    mtype_t* kptr0 = kptr;
    mtype_t* kptr1 = kptr+size;
    size_t vl;
    int32_t blkCnt = size;
    if (size < 32) {
        vl = 1;
        vint8m1_t  s8, k80, k81;
        vint32m1_t v_sum0 = __riscv_vmv_v_x_i32m1(0, vl);
        vint32m1_t v_sum1 = __riscv_vmv_v_x_i32m1(0, vl);
        for (; (vl = __riscv_vsetvl_e8m1(blkCnt)) > 0; blkCnt -= vl) {
            s8  = __riscv_vle8_v_i8m1(sptr, vl);
            sptr += vl;
            k80  = __riscv_vle8_v_i8m1(kptr0, vl);
            kptr0 += vl;
            k81  = __riscv_vle8_v_i8m1(kptr1, vl);
            kptr1 += vl;
            v_sum0 = __riscv_vwredsum_vs_i16m2_i32m1(__riscv_vwmul_vv_i16m2(s8, k80, vl), v_sum0, vl);
            v_sum1 = __riscv_vwredsum_vs_i16m2_i32m1(__riscv_vwmul_vv_i16m2(s8, k81, vl), v_sum1, vl);
        }
        sum0 = __riscv_vmv_x_s_i32m1_i32(v_sum0);
        sum1 = __riscv_vmv_x_s_i32m1_i32(v_sum1);
    } else {
        vl = 1;
        vint8m4_t  s8, k80, k81;
        vint32m1_t v_sum0 = __riscv_vmv_v_x_i32m1(0, vl);
        vint32m1_t v_sum1 = __riscv_vmv_v_x_i32m1(0, vl);
        for (; (vl = __riscv_vsetvl_e8m4(blkCnt)) > 0; blkCnt -= vl) {
            s8  = __riscv_vle8_v_i8m4(sptr, vl);
            sptr += vl;
            k80  = __riscv_vle8_v_i8m4(kptr0, vl);
            kptr0 += vl;
            k81  = __riscv_vle8_v_i8m4(kptr1, vl);
            kptr1 += vl;
            v_sum0 = __riscv_vwredsum_vs_i16m8_i32m1(__riscv_vwmul_vv_i16m8(s8, k80, vl), v_sum0, vl);
            v_sum1 = __riscv_vwredsum_vs_i16m8_i32m1(__riscv_vwmul_vv_i16m8(s8, k81, vl), v_sum1, vl);
        }
        sum0 = __riscv_vmv_x_s_i32m1_i32(v_sum0);
        sum1 = __riscv_vmv_x_s_i32m1_i32(v_sum1);
    }

    result[0] = sum0;
    result[1] = sum1;
    return;
}

TM_INLINE void tm_dot_prod_gap_3x3x1(mtype_t* sptr, mtype_t* kptr, uint32_t* k_oft, sumtype_t* result)
{
    *result = sptr[k_oft[0]]*kptr[0] + sptr[k_oft[1]]*kptr[1] + sptr[k_oft[2]]*kptr[2] + \
        sptr[k_oft[3]]*kptr[3] + sptr[k_oft[4]]*kptr[4] + sptr[k_oft[5]]*kptr[5] + \
        sptr[k_oft[6]]*kptr[6] + sptr[k_oft[7]]*kptr[7] + sptr[k_oft[8]]*kptr[8] ;
    return;
}

TM_INLINE void tm_dot_prod_3x3x1(mtype_t* sptr, mtype_t* kptr, sumtype_t* result)
{
    size_t vl = 9;
    vint8m1_t a0m1, b0m1;
    vint32m1_t v_zero0 = __riscv_vmv_v_x_i32m1(0, vl);
    a0m1  = __riscv_vle8_v_i8m1(sptr, vl);
    b0m1  = __riscv_vle8_v_i8m1(kptr, vl);
    v_zero0 = __riscv_vwredsum_vs_i16m2_i32m1(__riscv_vwmul_vv_i16m2(a0m1, b0m1, vl), v_zero0, vl);

    *result = __riscv_vmv_x_s_i32m1_i32(v_zero0);
    return;
}
#endif /* #if ENABLE_NUCLEI_EXT */

#else
#error "ERR MDL TYPE"
#endif

#if (TM_MDL_TYPE==TM_MDL_FP32)

TM_INLINE void tm_postprocess_sum(int n, sumtype_t* sums, btype_t* bs, int act, mtype_t* outp, \
    sctype_t* scales, sctype_t out_s, zptype_t out_zp)
{
    for(int i = 0; i < n; i++) {
        sumtype_t sum = sums[i];
        sum += bs[i];
        switch(act){    //activation func
        case TM_ACT_RELU:
        case TM_ACT_RELU6: //treat relu6 as relu in float mode //speed up
            sum = sum>0?sum:0;
            break;
        //    sum = sum>0?sum:0;
        //    sum = sum>6?6:sum;
        //    break;
        default:
            break;
        }
        outp[i] = (mtype_t)sum;
    }
    return;
}

#elif (TM_MDL_TYPE==TM_MDL_FP16)

TM_INLINE void tm_postprocess_sum(int n, sumtype_t* sums, btype_t* bs, int act, mtype_t* outp, \
    sctype_t* scales, sctype_t out_s, zptype_t out_zp)
{
    if(n < 4) {
        for(int i = 0; i < n; i++) {
            sumtype_t sum = sums[i];
            sum += bs[i];
            switch(act){    //activation func
            case TM_ACT_RELU:
            case TM_ACT_RELU6: //treat relu6 as relu in float mode //speed up
                sum = sum>0?sum:0;
                break;
            //    sum = sum>0?sum:0;
            //    sum = sum>6?6:sum;
            //    break;
            default:
                break;
            }
            outp[i] = (mtype_t)sum;
        }
    } else {
        size_t vl = __riscv_vsetvl_e16m2(n);
        vfloat16m2_t s16 = __riscv_vle16_v_f16m2(sums, vl);           //load f16
        vfloat16m2_t b16 = __riscv_vle16_v_f16m2(bs, vl);
        s16 = __riscv_vfadd_vv_f16m2(s16,b16,vl);                    //add bias

        switch(act){    //activation func
        case TM_ACT_RELU:
        case TM_ACT_RELU6:
            s16 = __riscv_vfmax_vf_f16m2(s16, 0.f, vl);
            break;
        default:
            break;
        }

        __riscv_vse16_v_f16m2(outp, s16, vl);
    }
    return;
}

#elif (TM_MDL_TYPE==TM_MDL_INT8) || (TM_MDL_TYPE==TM_MDL_INT16)

#if !TM_FASTSCALE
TM_INLINE void tm_postprocess_sum(int n, sumtype_t* sums, btype_t* bs, int act, mtype_t* outp, sctype_t* scales, sctype_t out_s_inv, zptype_t out_zp)
#else
TM_INLINE void tm_postprocess_sum(int n, sumtype_t* sums, btype_t* bs, int act, mtype_t* outp, int32_t* scales, int32_t out_s, zptype_t out_zp)
#endif
{
    #if !TM_FASTSCALE
    if(n < 4) {
    #endif
        for(int i = 0; i < n; i++) {
            sumtype_t sum = sums[i];
            sum += bs[i];
            #if !TM_FASTSCALE
                float sumf = sum*scales[i];
            #else
                sumtype_t sumf = (sum<<TM_FASTSCALE_SHIFT)/scales[i];
            #endif
            switch(act){    //activation func
            case TM_ACT_RELU:
                sumf = fmaxf(sumf, 0.f);
                break;
            case TM_ACT_RELU6:
                sumf = fmaxf(sumf, 0.f);
            #if (!TM_FASTSCALE)
                sumf = fminf(sumf, 6.f);
            #else
                sumf = sumf>(6<<TM_FASTSCALE_SHIFT)?(6<<TM_FASTSCALE_SHIFT):sumf;
            #endif
                break;
            default:
                break;
            }
            #if !TM_FASTSCALE
                outp[i] = (mtype_t)(sumf*out_s_inv + out_zp);  //(mtype_t)((int)(sumf/out_s) + out_zp) //(mtype_t)((int)(sumf/out_s +0.5) + out_zp)
            #else
                outp[i] = (mtype_t)(((sumf*out_s)>>(TM_FASTSCALE_SHIFT+TM_FASTSCALE_SHIFT))+out_zp);
            #endif
        }
    #if !TM_FASTSCALE
    } else {
        size_t vl = __riscv_vsetvl_e8m1(n);
        vint32m4_t s32 = __riscv_vle32_v_i32m4(sums, vl);           //load i32
        vint32m4_t b32 = __riscv_vle32_v_i32m4(bs, vl);
        s32 = __riscv_vadd_vv_i32m4(s32,b32,vl);                    //add bias

        vfloat32m4_t scalesf = __riscv_vle32_v_f32m4(scales, vl);   //load f32
        vfloat32m4_t sumsf = __riscv_vfcvt_f_x_v_f32m4(s32, vl);     //convert to float
        sumsf = __riscv_vfmul_vv_f32m4(sumsf, scalesf, vl);

        switch(act){    //activation func
        case TM_ACT_RELU:
            sumsf = __riscv_vfmax_vf_f32m4(sumsf, 0.f, vl);
            break;
        case TM_ACT_RELU6:
            sumsf = __riscv_vfmax_vf_f32m4(sumsf, 0.f, vl);
            sumsf = __riscv_vfmin_vf_f32m4(sumsf, 6.f, vl);
            break;
        default:
            break;
        }

        sumsf = __riscv_vfmul_vf_f32m4(sumsf, out_s_inv, vl);
        sumsf = __riscv_vfadd_vf_f32m4(sumsf, out_zp, vl);
        vint16m2_t s16 = __riscv_vfncvt_x_f_w_i16m2(sumsf, vl); // rvv0.7 doesn't support rtz
        vint8m1_t s8 = __riscv_vncvt_x_x_w_i8m1(s16, vl);
        __riscv_vse8_v_i8m1(outp, s8, vl);
    }
    #endif
    return;
}
#endif
