.altmacro
.arm

.text
.extern randoms
.extern out
.extern w
.extern wi
.extern d
.extern o
.extern s1
.extern s2
.extern s
.extern in
.extern tmp

.global sumarSenalesASM
.global multiplicarVentanasMonoASM
.global multiplicarVentanasStereoASM
.global getFlotasInt16ASM
.global multiplicarEspectrosASM
.global getIntsInt16_ditherASM
.global bucleMedioFftASM
.global splitMonoASM
.global bucleMedioSplitStereoASM
.global bucleEspectroMonoASM
.global rIFFTyaosASM_1
.global rIFFTyaosASM_2
.global rIFFTyaomASM_1
.global rIFFTyaomASM_2


.macro sumate_8_flotas

    vld1.32 {d0-d3}, [r0]!

    vadd.f32 d0, d0, d4
    vld1.32  d4, [r1]!

    vadd.f32 d1, d1, d5
    vld1.32  d5, [r1]!    

    vadd.f32 d2, d2, d6
    vld1.32  d6, [r1]!

    vadd.f32 d3, d3, d7
    vld1.32  d7, [r1]!
    vst1.32 {d0-d3}, [r3]!
.endm


.macro multiplica_8_flotas

    vld1.32 {d0-d3}, [r0]!

    vmul.f32 d0, d0, d4
    vld1.32  d4, [r1]!

    vmul.f32 d1, d1, d5
    vld1.32  d5, [r1]!    

    vmul.f32 d2, d2, d6
    vld1.32  d6, [r1]!

    vmul.f32 d3, d3, d7
    vld1.32  d7, [r1]!
    vst1.32 {d0-d3}, [r3]!
.endm


sumarSenalesASM:
    /*asumimos que la cantidad de elemenos a procesar
    es divisible por 16 (de hecho lo re es porque lo
    elegimos nosotros y no nos trae ningun porblema
    hacerlo) */

    STMFD   sp!, {r3,fp,ip,lr}

    mov  r3, r0
    subs r2, r2, #16        /* ¿fix? para no crear segfault */
    vld1.32 {d4-d7},[r1]!       /* sum_8_flot asume d4-d7 esta cargado */

    .loopSumaSenales:

    sumate_8_flotas    
    sumate_8_flotas

    /* ahora quedan 16 menos*/
        SUBS        r2, r2, #16

    BNE .loopSumaSenales

    /* faltan 16 que ya estan cargados en {d4-d7} */    
    
    vld1.32 {d0-d3},[r0]!
    vadd.f32 d0, d0, d4
    vadd.f32 d1, d1, d5
    vadd.f32 d2, d2, d6
    vadd.f32 d3, d3, d7
    vst1.32 {d0-d3},[r3]!

    LDMFD   sp!, {r3,fp,ip,lr}
    BX      lr



multiplicarVentanasMonoASM:

    STMFD   sp!, {r3,fp,ip,lr}

    mov  r3, r0
    subs r2, r2, #16        /* ¿fix? para no crear segfault */
    vld1.32 {d4-d7},[r1]!       /* mul_8_flot asume d4-d7 esta cargado */

   .loopMultiplicarVentanasMono:

    multiplica_8_flotas
    multiplica_8_flotas
        subs r2, r2, #16

    BNE .loopMultiplicarVentanasMono

    vld1.32 {d0-d3},[r0]!
    vmul.f32 d0, d0, d4
    vmul.f32 d1, d1, d5
    vmul.f32 d2, d2, d6
    vmul.f32 d3, d3, d7
    vst1.32 {d0-d3},[r3]!

    LDMFD   sp!, {r3,fp,ip,lr}
    BX      lr


multiplicarVentanasStereoASM:

    STMFD   sp!, {fp,ip,lr}

    #r0 = v1
    #r1 = v2
    #r2 = tamano

    lsl r2, #1

   .loopMultiplicarVentanasStereo:

	vld2.32 {q0,q1}, [r0]
        vld1.32 {q2}   , [r1]!
	
	vmul.f32 q0, q0, q2
	vmul.f32 q1, q1, q2

	vst2.32 {q0,q1}, [r0]!
	
        subs r2, r2, #8
    	bne .loopMultiplicarVentanasStereo


    LDMFD   sp!, {fp,ip,lr}
    BX      lr



getFlotasInt16ASM:

    STMFD   sp!, {fp,ip,lr}
    /*en r2 tengo el tamano del arreglo*/

    ldr r3, =(wi)
    vld1.f32 d0[0], [r3]    /* copiamos wi en q0 */
    vmov s1, s0
    vmov s2, s0
    vmov s3, s0
    
    
    .loopGetFlotasInt16:

    vld1.64  d2, [r0]!    /* levantamos 4 sig16*/
    vmovl.s16 q1, d2    /* desepanque en q1 */
    vcvt.f32.s32 q1, q1    /* convertimos en f32 */
    
    vmul.f32 q1, q1, q0
    vst1.64 {d2-d3}, [r1]!

    subs r2, r2, #4

    BNE .loopGetFlotasInt16


    LDMFD   sp!, {fp,ip,lr}
    BX      lr


multiplicarEspectrosASM:
    
    STMFD   sp!, {fp,ip,lr}
    mov r3, r0

    /* r0, r1 arreglos COMPLEX */
    /* r2  tamanio */ 
    subs r2, r2, #1    /* fix malisimo para hacerlo par */

    .loopMultiplicarComplex:

    /* [r0] = [a,b,a,b] */
    /* [r1] = [c,d,c,d] */

    vld2.32 {d0,d1},[r0]!   /* d0 = [a,a], d1 = [b,b] */
    vld2.32 {d2,d3},[r1]!   /* d2 = [c,c], d3 = [d,d] */
    
    vmul.f32 d4, d0, d2     /* d4 = ac */
    vmls.f32 d4, d1, d3     /* d4 = ac - bd */

    vmul.f32 d5, d0, d3     /* d5 = ad */
    vmla.f32 d5, d1, d2     /* d5 = ad + bc */

    /* [d4] = [r0,r1] */ 
    /* [d5] = [i0,i1] */

    vst2.32 {d4,d5}, [r3]! /* [r3] = [r0,i0,r1,i1] */
    
        SUBS    r2, r2, #2

    BNE .loopMultiplicarComplex

        vld2.32 {d0[0],d1[0]} ,[r0]!    
        vld2.32 {d2[0],d3[0]} ,[r1]!    
         
        vmul.f32 d4, d0, d1     /* d4 = [ac, bd] */ 
        vmls.f32 d4, d1, d3     /* d4 = ac - bd */ 
 
        vmul.f32 d5, d0, d3     /* d5 = ad */ 
        vmla.f32 d5, d1, d2     /* d5 = ad + bc */ 
 
        /* [d4] = [r0,r1] */  
        /* [d5] = [i0,i1] */ 
 
        vst2.32 {d4[0],d5[0]}, [r3]! /* [r3] = [r0,i0,r1,i1] */


    LDMFD   sp!, {fp,ip,lr}
    BX      lr


getIntsInt16_ditherASM:
    
    /* parametros pasados son (en orden):
       r0= ints     (arreglo de int16_t para devolver)
       r1= flotas   (arreglo de floats, entrada)
       r2= tamano   (cantidad de ints y flotas)
     */

    /* guardo r4 */
    STMFD   sp!, {r4,fp,ip,lr}

    /* cargo la variable s1 en (justamente) el registro s1 */
    LDR         r3, =(s1)
    VLD1.f32    d0[1], [r3]
    /* cargo la variable s2 en (justamente) el registro s2 */
    LDR         r3, =(s2)
    VLD1.f32    d1[0], [r3]
    /* cargo la variable s en el refistro s4 */
    LDR         r3, =(s)
    VLD1.f32    d2[0], [r3]
    /* cargo la variable w en el registro s6 */
    LDR         r3, =(w)
    VLD1.f32    d3[0], [r3]
    /* cargo la variable wi en el registro s7 */
    LDR         r3, =(wi)
    VLD1.f32    d3[1], [r3]
    /* cargo la variable o en el registro s8 */
    LDR         r3, =(o)
    VLD1.f32    d4[0], [r3]
    /* cargo la variable d en el registro s9 */
    LDR         r3, =(d)
    VLD1.f32    d4[1], [r3]
    /* randoms es un puntero a un arreglo de floats, enotnces en r3
       tengo un puntero a un puntero.*/
    LDR         r3, =(randoms)
    /* ahora paso el puntero al arreglo a r3 */
    LDR         r3, [r3]


    /* recordemos que hay en cada registro:
       s0= in
       s1= s1
       s2= s2
       s3=
       s4= s
       s5= tmp
       s6= w
       s7= wi
       s8= o
       s9= d
       s10= rand

       r0= ints     (arreglo de int16_t para devolver)
       r1= flotas   (arreglo de floats, entrada)
       r2= tamano   (cantidad de ints y flotas)
       r4= out
    */

    .loopGetIntsInt16_dither:


#        in = flotas[i];
        VLD1.f32    d0[0], [r1]!

#        in += s * (s1 + s1 - s2);            //error feedback
        VMLS.f32    s0, s4, s2
        VADD.f32    s0, s0, s1

#        tmp = in + o + d * randoms[i];    //dc offset and dither
        VADD.f32    s5, s0, s8
        /* traermos un rand a s10 */
        VLD1.f32    d5[0], [r3]!
        VMLA.f32    s5, s9, s10

#        out = (int16_t)(w * tmp);            //truncate downwards
        VMUL.f32    s5, s5, s6

        #        s2 = s1;
        #        es mejor poner esto intercalado
            VMOV.f32    s2, s1

        VCVT.s32.f32    s5, s5
        VMOV        r4, s5
#        if(tmp<0.0) out--;                  //this is faster than floor()
# mejor hacer esto en NEON?
        SUBMI       r4, r4, #1
#        /* recien aca termino la conversion a halfword */
#         esto no hace falta porque lo puedo guardar direactamente
#        UXTH        r4, r4


        #        //devolvemos resultado
        #        ints[i] = out;
        #        es mejor poner esto intercalado
                STRH        r4, [r0], #2


#        s1 = in - wi * (float)out;           //error
        VMOV        s5, r4
        VCVT.f32.s32    s5, s5
        VMOV.f32    s1, s0
        VMLS.f32    s1, s7, s5


#    #################  UNROLING... segunda pasada
#
#        VLD1.f32    d0[0], [r1]!
#
#        VMLS.f32    s0, s4, s2
#        VADD.f32    s0, s0, s1
#
#        VADD.f32    s5, s0, s8
#        VLD1.f32    d5[0], [r3]!
#        VMLA.f32    s5, s9, s10
#
#        VMUL.f32    s5, s5, s6
#        VCVT.s32.f32    s5, s5
#        VMOV        r4, s5
#        SUBMI       r4, r4, #1
#
#        STRH        r4, [r0], #2
#
#
#        VMOV.f32    s2, s1
#        VMOV        s5, r4
#        VCVT.f32.s32    s5, s5
#        VMOV.f32    s1, s0
#        VMLS.f32    s1, s7, s5



        SUBS        r2, r2, #1

    BNE .loopGetIntsInt16_dither

    /* guardo s1 */
    LDR         r3, =(s1)
    VST1.f32    d0[1], [r3]
    /* guardo s2 */
    LDR         r3, =(s2)
    VST1.f32    d1[0], [r3]


    LDMFD   sp!, {r4,fp,ip,lr}
    BX      lr



bucleMedioFftASM:

    STMFD   sp!, {r4-r10,fp,ip,lr}

    #bucleMedioFftASM(pares, salto, resAct1, resAct2, ti, tr)
    #r0 = &pares, r1 = salto. r2 = &resAct1, r3 = &resAct2, pila1 = ti, pila2 = tr
    

    ldr r7, [r0]
    lsl r6, r1, #3
    ldr r9, [r2]
    ldr r10, [r3]
    ldr r4, [sp, #(0+40)]
    ldr r5, [sp, #(4+40)]
   
    #asi quedaron los registros
    #r0 = &pares
    #r1 = j
    #r2 = &resActual1
    #r3 = &resActual2
    #r4 = &ti
    #r5 = &tr
    #r6 = salto en bytes
    #r7 = pares
    #r8 = impares
    #r9 = resAct1
    #r10 = resAct2

    vld1.f32 d0[0], [r4]
    vdup.32  q0, d0[0]
    # q0:[TI,TI,TI,TI]

    vld1.f32 d2[0], [r5]
    vdup.32  q1, d2[0] 
    # q1:[TR,TR,TR,TR]
 
    .loop_j_fft:

        add r8, r7, r6
        #impar = par + salto

        cmp r1, #4
        blt .deAdos

        .deAcuatro:
 
            vld2.32 {q2,q3}, [r8]
                                    #accedo a impar       
                                    #q2 tiene reales
                                    #q3 tiene imaginarios

            vmul.f32 q12, q2, q1      
                                    # impares.r * TR
                                    # q12: q2 * TR
            vmls.f32 q12, q3, q0      
                                    # q12:iRtemp: q2*TR - q3*TI
            vmul.f32 q13, q2, q0      
                                    # q13: q2 * TI
            vmla.f32 q13, q3, q1 
                                    # q13:iItemp: q2*TI + q3*TI

            vld2.32 {q10,q11}, [r7]!
                                    # q10,q11: pares.r,pares.i
                                    # q10 tiene realies
                                    # q11 tiene imaginarios

            vadd.f32 q2, q10, q12     
                                    #resAct1.r = pares.r + iRtmp
            vadd.f32 q3, q11, q13     
                                    #resAct1.i = pares.i + iItmp
            vst2.32 {q2,q3}, [r9]!
                                    #guardo resAct1 en memoria

            vsub.f32 q10, q10, q12    
                                    #resAct2.r = pares.r - iRtmp
            vsub.f32 q11, q11, q13    
                                    #resAct2.i = pares.i - iItmp
            vst2.32 {q10,q11}, [r10]!
                                    #guardo resAct2 en memoria

            subs r1, #4
            bne .loop_j_fft
            b .fin

        .deAdos:

            cmp r1, #2
            blt .deAuno

            vld2.32 {d4,d6}, [r8]
                                    #accedo a impar       
            vmul.f32 q12, q2, q1      
                                    # q12: q2 * TR
            vmls.f32 q12, q3, q0      
                                    # q12:iRtemp: q2*TR - q3*TI
            vmul.f32 q13, q2, q0      
                                    # q13: q2 * TI
            vmla.f32 q13, q3, q1 
                                    # q13:iItemp: q2*TI + q3*TI
            vld2.32 {d20,d22}, [r7]!
                                    # q10,q11: pares.r,pares.i
            vadd.f32 q2, q10, q12     
                                    #resAct1.r = pares.r + iRtmp
            vadd.f32 q3, q11, q13     
                                    #resAct1.i = pares.i + iItmp
            vst2.32 {d4,d6},  [r9]!
                                    #guardo resAct1 en memoria
            vsub.f32 q10, q10, q12    
                                    #resAct2.r = pares.r + iRtmp
            vsub.f32 q11, q11, q13    
                                    #resAct2.i = pares.i - iItmp
            vst2.32 {d20,d22}, [r10]!
                                    #guardo resAct2 en memoria

            subs r1, #2
            bne .loop_j_fft
            b .fin


        .deAuno:

            vld2.32 {d4[0],d6[0]}, [r8]
                                    #accedo a impar       
            vmul.f32 q12, q2, q1      
                                    # q12: q2 * TR
            vmls.f32 q12, q3, q0      
                                    # q12:iRtemp: q2*TR - q3*TI
            vmul.f32 q13, q2, q0      
                                    # q13: q2 * TI
            vmla.f32 q13, q3, q1 
                                    # q13:iItemp: q2*TI + q3*TI
            vld2.32 {d20[0],d22[0]}, [r7]!
                                    # q10,q11: pares.r,pares.i
            vadd.f32 q2, q10, q12     
                                    #resAct1.r = pares.r + iRtmp
            vadd.f32 q3, q11, q13     
                                    #resAct1.i = pares.i + iItmp
            vst2.32 {d4[0],d6[0]},  [r9]!
                                    #guardo resAct1 en memoria
            vsub.f32 q10, q10, q12    
                                    #resAct2.r = pares.r + iRtmp
            vsub.f32 q11, q11, q13    
                                    #resAct2.i = pares.i - iItmp
            vst2.32 {d20[0],d22[0]}, [r10]!
                                    #guardo resAct2 en memoria

            subs r1, #1
            bne .loop_j_fft

    .fin:

    #tengo que guardar pares, reasAct1 y resAct2

    str r7, [r0]
    str r9, [r2]
    str r10, [r3]


    LDMFD   sp!, {r4-r10,fp,ip,lr}
    BX      lr


splitMonoASM:
    STMFD   sp!, {r4,r5,r6,fp,ip,lr}

    #r0 = complex *X
    #r1 = complex *A
    #r2 = complex *B
    #r3 = complex *espectro
    #r4 = complex *X^-1
    #r5 = longitud
    #r6 = -8

    ldr r5, [sp,#(0+24)]
    
    lsl r4, r5, #3
    add r4, r0

    #mirar con cuidado el codigo en C y
    #convencerse que aca va un 3
    sub r4, #(8*3)
    mov r6, #(-8*4)

    .loop_k_splitMono:
 
        vld2.32 {q12,q13}, [r4], r6
        #X[longitud-k]
        #falta suapear

        vrev64.32  q12,  q12
        vswp      d24, d25  
        #invertimos el orden q12

        vld2.32 {q0,q1}, [r0]!
        #X[k]

        vrev64.32  q13, q13
        vswp      d26,d27
        #invertimos el orden q13

        vld2.32 {q2,q3}, [r1]!
        #A[k]

        vld2.32 {q10,q11}, [r2]!
        #B[k]

        vmul.f32 q14, q0, q2
        #q14 = X[k].r * A[k].r
        vmls.f32 q14, q1, q3
        #q14 = q14 - X[k].i * A[k].i 
        vmla.f32 q14, q12, q10
        #q14 = q14 + X[l-k].r * B[k].r
        vmla.f32 q14, q13, q11
        #q14 = q14 + X[l-k].i * B[k].i
        
        vmul.f32 q15, q1, q2
        #q15 = X[k].i * A[k].r
        vmla.f32 q15, q0, q3
        #q15 = q15 + X[k].r * A[k].i 
        vmla.f32 q15, q12, q11
        #q15 = q15 + X[l-k].r * B[k].i
        vmls.f32 q15, q13, q10
        #q15 = q15 - X[l-k].i * B[k].r

        vst2.32 {q14, q15}, [r3]!

        subs r5, #4
        bne .loop_k_splitMono
        
    LDMFD   sp!, {r4,r5,r6,fp,ip,lr}
    BX      lr


bucleMedioSplitStereoASM:
    STMFD   sp!, {r4,r5,r6,r7,r8,fp,ip,lr}

    #r0 = * espectroL
    #r1 = * espectroR
    #r2 = * X
    #r3 = longitud

    vmov.f32 s0, #0.5
    vdup.f32 q10, d0[0]
    #q10 = 0.5, 0.5, 0.5, 0.5

    lsr r4, r3, #1
    # r4: N = longitud / 2

    lsl r3, #3

      add r5, r2, r3
      sub r5, #(8*3)    
      #r5 = X[l-r]

      add r6, r0, r3
      sub r6, #(8*3)    
      #r6 = espectroL[l-r]

      add r7, r1, r3
      sub r7, #(8*3)    
      #r7 = espectroR[l-r]

    lsr r3, #3

    mov r8, #(-8*4)

    .loop_k_splitStereo:
 
       vld2.32 {q0,q1}, [r2]!
       #X[k]
        
       vld2.32 {q2,q3}, [r5], r8
       
         vrev64.32  q2, q2
         vswp       d4, d5  

         vrev64.32  q3, q3
         vswp       d6, d7  
       #X[l-k]

       vadd.f32 q11, q0, q2
       vmul.f32 q11, q11, q10
       #q11 = (X[k].r + X[l-k].r) / 2

       vsub.f32 q12, q1, q3
       vmul.f32 q12, q12, q10
       #q12 = (X[k].i - X[l-k].i) / 2

       vadd.f32 q13, q1, q3
       vmul.f32 q13, q13, q10
       #q13 = (X[k].i + X[l-k].i) / 2

       vsub.f32 q14, q2, q0
       vmul.f32 q14, q14, q10
       #q14 = (X[l-k].r - X[k].r) / 2

       vst2.32 {q11,q12}, [r0]!
       vst2.32 {q13,q14}, [r1]!

       vrev64.32  q11,  q11
       vswp      d22, d23  

       vrev64.32  q12,  q12
       vswp      d24, d25  

       vneg.f32 q12, q12

       vrev64.32  q13,  q13
       vswp      d26, d27  

       vrev64.32  q14,  q14
       vswp      d28, d29  

       vneg.f32 q14, q14
      
       vst2.32 {q11,q12}, [r6], r8
       vst2.32 {q13,q14}, [r7], r8

       subs r4, #4
       bne .loop_k_splitStereo


    
    LDMFD   sp!, {r4,r5,r6,r7,r8,fp,ip,lr}
    BX      lr


bucleEspectroMonoASM:
    STMFD   sp!, {fp,ip,lr}

    #r0 = espectro
    #r1 = longitud

    add r2, r0, r1, lsl #4
    sub r2, #(8*3)
    mov r3, #(-8*4)

    #r2 = espectro[2*longitud]

    .loop_k_espectroModo:
       vld2.32  {q0,q1}, [r0]!
       vneg.f32  q1, q1
       vst2.32  {q0,q1}, [r2], r3

       subs r1, #4
       bne .loop_k_espectroModo


    LDMFD   sp!, {fp,ip,lr}
    BX      lr


rIFFTyaomASM_1:
    
    STMFD   sp!, {fp,ip,lr}
    
    #r0 = temporalComplex
    #r1 = longitud
    
    .loop_k_yaomasm_1:

      vld2.32 {q0,q1}, [r0]
      vneg.f32 q1, q1
      vst2.32 {q0,q1}, [r0]!

      subs r1, #4
      bne .loop_k_yaomasm_1
    
    LDMFD   sp!, {fp,ip,lr}
    BX      lr

rIFFTyaomASM_2:
    
    STMFD   sp!, {fp,ip,lr}

    #r0 = x
    #r1 = longitud    
    vmov     s0, r1
    vcvt.f32.s32 s0 ,s0
    vmov.f32 s1, #1.0
    vdiv.f32 s0, s1, s0
    vdup.f32 q10, d0[0]

    #q10 = 1/longitud

    .loop_k_yaomasm_2:

      vld2.32 {q0,q1}, [r0]

      vmul.f32 q0, q0, q10
      vmul.f32 q1, q1, q10
      vneg.f32 q1, q1
      vst2.32 {q0,q1}, [r0]!

      subs r1, #4
      bne .loop_k_yaomasm_2


    LDMFD   sp!, {fp,ip,lr}
    BX      lr


rIFFTyaosASM_1:
    
    STMFD   sp!, {fp,ip,lr}

    #r0 = x
    #r1 = espectroL
    #r2 = espectroR     
    #r3 = longitud

    .loop_k_yaosasm_1:
       
       vld2.32 {q0,q1}, [r1]!
       #espectroL
       vld2.32 {q2,q3}, [r2]!
       #espectroR 
       
       vsub.f32 q10, q0, q3
       vadd.f32 q11, q1, q2
       vneg.f32 q11, q11

       vst2.32 {q10,q11}, [r0]!       

       subs r3, #4
       bne .loop_k_yaosasm_1	

    LDMFD   sp!, {fp,ip,lr}
    BX      lr

rIFFTyaosASM_2:
    
    STMFD   sp!, {fp,ip,lr}

    #r0 = x
    #r1 = temporalComplex
    #r2 = longitud    
    vmov     s0, r2
    vcvt.f32.s32 s0 ,s0
    vmov.f32 s1, #1.0
    vdiv.f32 s0, s1, s0
    vdup.f32 q10, d0[0]

    #q10 = 1/longitud
    

    .loop_k_yaosasm_2:

      vld2.32 {q0,q1}, [r1]!
      vmul.f32 q0, q10
      vmul.f32 q1, q10
      vst2.32 {q0,q1}, [r0]!

      subs r2, #4
      bne .loop_k_yaosasm_2

    LDMFD   sp!, {fp,ip,lr}
    BX      lr

