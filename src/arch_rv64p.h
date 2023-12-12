#include "stdlib.h"
#include "stdint.h"
#include "math.h"
#include "tm_port.h"

#define ENABLE_NUCLEI_EXT 1

#if TM_MDL_TYPE == TM_MDL_INT8

#if ENABLE_NUCLEI_EXT
TM_INLINE void tm_dot_prod(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{
    int32_t sum;
    asm  volatile(
    "mv      t1,%[size]            \n"
    "mv      a1,%[kptr]            \n"
    "mv      a0,%[sptr]            \n"
    "srai    t2,t1,3               \n"
    "li      t6,0                  \n"
    "beqz    t2,LOOPEND%=\n        \n"
    "LOOP%=: \n"
    "ld      t5,0(a0)              \n"
    "addi    a0,a0,8               \n"
    "ld      a7,0(a1)              \n"
    "addi    a1,a1,8               \n"
    "smaqa   t6,t5,a7              \n"
    "addi    t2,t2,-1              \n"
    "bnez    t2,LOOP%=             \n"
    "srai    t4,t6,0x20            \n"
    "andi    t1,t1,7               \n"
    "addw    t6,t6,t4              \n"
    "LOOPEND%=:\n                  \n"
    "beqz    t1,OUT%=              \n"
    "add     a7,a0,t1              \n"
    "REST%=: \n"
    "lb      t4,0(a0)              \n"
    "addi    a0,a0,1               \n"
    "lb      t1,0(a1)              \n"
    "addi    a1,a1,1               \n"
    "maddr32 t6,t1,t4              \n"
    "bne     a7,a0,REST%=          \n"
    "OUT%=: \n"
    "mv      %[sum],t6            \n"
    :[sum]"=r"(sum)
    :[sptr]"r"(sptr),[kptr]"r"(kptr),[size]"r"(size)
    :"t1", "t2", "a0", "a1", "t4", "t5", "t6", "a7"
    );

    *result = sum;
    return;
}

TM_INLINE void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{
    int32_t sum0, sum1;
    asm  volatile(
    "mv      a2,%[size]            \n"
    "mv      a1,%[kptr]            \n"
    "mv      a0,%[sptr]            \n"
    "srai    t1,a2,3               \n"
    "add     a6,a1,a2              \n"
    "li      t0,0                  \n"
    "li      t6,0                  \n"
    "beqz    t1,LOOPEND%=\n        \n"
    "mv      t2,t1                 \n"
    "LOOP%=: \n"
    "ld      t4,0(a0)              \n"
    "addi    a0,a0,8               \n"
    "ld      t3,0(a1)              \n"
    "addi    a1,a1,8               \n"
    "ld      t5,0(a6)              \n"
    "addi    a6,a6,8               \n"
    "smaqa   t6,t4,t3              \n"
    "smaqa   t0,t4,t5              \n"
    "addi    t2,t2,-1              \n"
    "bnez    t2,LOOP%=\n"
    "srai    a3,t0,0x20            \n"
    "srai    a7,t6,0x20            \n"
    "andi    a2,a2,7               \n"
    "addw    t0,t0,a3              \n"
    "addw    t6,t6,a7              \n"
    "LOOPEND%=:\n                  \n"
    "beqz    a2,OUT%=              \n"
    "add     t3,a0,a2              \n"
    "REST%=: \n"
    "lb      t5,0(a0)              \n"
    "addi    a0,a0,1               \n"
    "lb      t1,0(a1)              \n"
    "addi    a1,a1,1               \n"
    "lb      t2,(a6)               \n"
    "addi    a6,a6,1               \n"
    "maddr32 t6,t1,t5              \n"
    "maddr32 t0,t2,t5              \n"
    "bne     t3,a0,REST%=          \n"
    "OUT%=: \n"
    "mv      %[sum0],t6            \n"
    "mv      %[sum1],t0            \n"
    :[sum0]"=r"(sum0),[sum1]"=r"(sum1)
    :[size]"r"(size),[kptr]"r"(kptr),[sptr]"r"(sptr)
    :"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a6", "a7"
    );

    result[0] = sum0;
    result[1] = sum1;
    return;
}

TM_INLINE void tm_dot_prod_3x3x1(mtype_t* sptr, mtype_t* kptr, sumtype_t* result)
{
    *result = sptr[0]*kptr[0] + sptr[1]*kptr[1] + sptr[2]*kptr[2] + \
        sptr[3]*kptr[3] + sptr[4]*kptr[4] + sptr[5]*kptr[5] + \
        sptr[6]*kptr[6] + sptr[7]*kptr[7] + sptr[8]*kptr[8] ;
    return;
}
 
TM_INLINE void tm_dot_prod_gap(mtype_t* sptr, mtype_t* kptr, uint32_t* k_oft, uint32_t size, sumtype_t* result)
{
    sumtype_t sum=0;
    uint32_t i = 0;
    uint32_t cnt = (size>>3)<<3;  //8
    for(; i+8-1 <cnt;){
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
        sum += sptr[k_oft[i]]*kptr[i]; i++;
    }
    for(; i <size; i++){
        sum += sptr[k_oft[i]]*kptr[i]; 
    }
    *result = sum;
    return;
}

TM_INLINE void tm_dot_prod_gap_3x3x1(mtype_t* sptr, mtype_t* kptr, uint32_t* k_oft, sumtype_t* result)
{
    *result = sptr[k_oft[0]]*kptr[0] + sptr[k_oft[1]]*kptr[1] + sptr[k_oft[2]]*kptr[2] + \
        sptr[k_oft[3]]*kptr[3] + sptr[k_oft[4]]*kptr[4] + sptr[k_oft[5]]*kptr[5] + \
        sptr[k_oft[6]]*kptr[6] + sptr[k_oft[7]]*kptr[7] + sptr[k_oft[8]]*kptr[8] ;
    return;                  
}

#else
#error "standard RV64P is not implement yet!"
#endif /* if ENABLE_NUCLEI_EXT */

#else
#error "RV64P opt for FP32 in not implement yet!"
#endif /* if TM_MDL_TYPE == TM_MDL_INT8 */

#if (TM_MDL_TYPE==TM_MDL_FP32) || (TM_MDL_TYPE==TM_MDL_FP16)

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

#elif (TM_MDL_TYPE==TM_MDL_INT8) || (TM_MDL_TYPE==TM_MDL_INT16)

#if !TM_FASTSCALE
TM_INLINE void tm_postprocess_sum(int n, sumtype_t* sums, btype_t* bs, int act, mtype_t* outp, sctype_t* scales, sctype_t out_s_inv, zptype_t out_zp)
#else
TM_INLINE void tm_postprocess_sum(int n, sumtype_t* sums, btype_t* bs, int act, mtype_t* outp, int32_t* scales, int32_t out_s, zptype_t out_zp)
#endif
{
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
            sumf = sumf>0?sumf:0;
            break;
        case TM_ACT_RELU6:
            sumf = sumf>0?sumf:0;
        #if (!TM_FASTSCALE)
            sumf = sumf>6?6:sumf;
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
    return;
}
#endif
