#include "stdlib.h"
#include "stdint.h"
#include "math.h"
#include "tm_port.h"

#define ENABLE_NUCLEI_EXT 1

#if TM_MDL_TYPE == TM_MDL_INT8

#if ENABLE_NUCLEI_EXT
TM_INLINE void tm_dot_prod(mtype_t* sptr, mtype_t* kptr,uint32_t size, sumtype_t* result)
{
    int32_t sum;
    asm  volatile(
    "srai    t4,%[size],0x3        \n"
    "beqz    t4,OUT%=  \n"
    "mv      a6,%[kptr]            \n"
    "mv      a4,%[sptr]            \n"
    "mv      a5,t4                 \n"
    "li      a7,0                  \n"
    "LOOP%=:  \n"
    "ld      t1,0(a4)              \n"
    "addi    a4,a4,8               \n"
    "ld      t3,0(a6)              \n"
    "addi    a6,a6,8               \n"
    "smaqa   a7,t1,t3              \n"
    "addi    a5,a5,-1              \n"
    "bnez    a5,LOOP%=  \n"
    "slli    a5,a7,0x20            \n"
    "slli    t4,t4,0x3             \n"
    "add     a7,a7,a5              \n"
    "add     a1,%[kptr],t4         \n"
    "add     a0,%[sptr],t4         \n"
    "srai    a7,a7,0x20            \n"
    "andi    a2,%[size],7          \n"
    "beqz    a2,OUT%=  \n"
    "REST%=: \n"
    "lb      t1,0(a1)              \n"
    "mv      a5,a7                 \n"
    "lb      a6,0(a0)              \n"
    "addi    a4,a2,-1              \n"
    "maddr32 a5,t1,a6              \n"
    "beqz    a4,OUT%=  \n"
    "lb      a7,1(a0)              \n"
    "addi    a4,a2,-2              \n"
    "lb      a6,1(a1)              \n"
    "maddr32 a5,a7,a6              \n"
    "beqz    a4,OUT%=  \n"
    "lb      a7,2(a0)              \n"
    "addi    a4,a2,-3              \n"
    "lb      a6,2(a1)              \n"
    "maddr32 a5,a7,a6              \n"
    "beqz    a4,OUT%=  \n"
    "lb      a7,3(a0)              \n"
    "addi    a4,a2,-4              \n"
    "lb      a6,3(a1)              \n"
    "maddr32 a5,a7,a6              \n"
    "beqz    a4,OUT%=  \n"
    "lb      a6,4(a0)              \n"
    "addi    a2,a2,-5              \n"
    "lb      a4,4(a1)              \n"
    "maddr32 a5,a6,a4              \n"
    "beqz    a2,OUT%=  \n"
    "lb      a7,5(a0)              \n"
    "li      a4,1                  \n"
    "lb      a6,5(a1)              \n"
    "maddr32 a5,a7,a6              \n"
    "beq     a2,a4,OUT%=  \n"
    "lb      a2,6(a0)              \n"
    "lb      a4,6(a1)              \n"
    "maddr32 a5,a2,a4              \n"
    "OUT%=: \n"
    "mv %[sum], a5         \n"
    :[sum]"=r"(sum)
    :[sptr]"r"(sptr),[kptr]"r"(kptr),[size]"r"(size)
     :"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a4", "a5", "a6", "a7"
    );

    *result = sum;
    return;
}

TM_INLINE  void tm_dot_prod_pack2(mtype_t* sptr, mtype_t* kptr, uint32_t size, sumtype_t* result)
{
    int32_t sum0, sum1;
    asm  volatile(
    "slli    a6,%[size],0x20 \n"
    "srliw   a5,%[size],0x3  \n"
    "srli    a6,a6,0x20      \n"
    "mv      t2,a5           \n"
    "add     a6,a6,%[kptr]   \n"
    "beqz    a5,OUT%= \n"
    "mv      t4,a6       \n"
    "mv      t3,%[kptr]  \n"
    "mv      t1,%[sptr]  \n"
    "mv      a7,a5       \n"
    "li      t6,0        \n"
    "li      t0,0        \n"
    "LOOP%=: \n"
    "ld      t5,0(t1)    \n"
    "addi    t1,t1,8     \n"
    "ld      a4,0(t3)    \n"
    "addi    t3,t3,8     \n"
    "ld      a5,0(t4)    \n"
    "addi    t4,t4,8     \n"
    "smaqa   t0,t5,a4    \n"
    "smaqa   t6,t5,a5    \n"
    "addi    a7,a7,-1    \n"
    "bnez    a7,LOOP%= \n"
    "slli    a5,t0,0x20  \n"
    "slli    a4,t6,0x20  \n"
    "slli    a7,t2,0x3   \n"
    "add     a5,a5,t0    \n"
    "add     a4,a4,t6    \n"
    "add     a6,a6,a7    \n"
    "add     a1,%[kptr],a7\n"
    "add     a0,%[sptr],a7\n"
    "srai    a5,a5,0x20  \n"
    "srai    a4,a4,0x20  \n"
    "andi    a7,%[size],7   \n"
    "beqz    a7,OUT%=   \n"
    "add     a7,a7,a0    \n"
    "REST%=: \n"
    "lb      a2,0(a0)    \n"
    "addi    a0,a0,1     \n"
    "lb      t3,0(a1)    \n"
    "addi    a6,a6,1     \n"
    "lb      t1,-1(a6)   \n"
    "addi    a1,a1,1     \n"
    "maddr32 a5,t3,a2    \n"
    "maddr32 a4,t1,a2    \n"
    "bne     a0,a7,REST%=\n"
    "OUT%=: \n"
    "mv %[sum0], a5\n"
    "mv %[sum1], a4\n"
    :[sum0]"=r"(sum0),[sum1]"=r"(sum1)
    :[sptr]"r"(sptr),[kptr]"r"(kptr),[size]"r"(size)
    :"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a4", "a5", "a6", "a7"
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
