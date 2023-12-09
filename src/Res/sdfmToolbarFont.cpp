#include "sdfmToolbarFont.h"

static const char SDFMT_compressed_data_base85[6680 + 1] =
    "7])#######S3#ca'/###O@?>#+lQS%Ql#v#X^@iFLnx%nK=%##=-&##ZSV=B*/NYG3KEe-]br-$GQshF?[]Y#^SSk+8=bw'fFu0FhE9##tIkA#04@m/)e@UCO6IS#;hG<-87T;-QF%U."
    "KY)W7RgG<-=^rS%MZIkEX>8NDjg'##%u(##bgD0FZr][Npk###S]'##hkGkOP&###dpv0hodPirgZl'A#ea1#J,Y:v>Lc>#JNit-].=GMM%g+Mc[@`aT.>_8:Ld--KMU;$]wu=PKGqW."
    "jc($#g3+V.%####(;#s-Pi[+M+DgV-tPJ=#IAuu#62cp&O=?5/N1X#GwPj)#A:h2qmQS5q:q+6qJKu6qdV[8q&6/Pq_$+xq6#thqNsDsq*qm$r07m/r:6s8r#X7`rh1IOr3kKYrn.V$s"
    "aualr<U2or['V?s0g6Ks<ZPbs&i>js)Y=MtKvtitwFbAuu'FxuC*>>#KFKF-kA,(.I;YN[$,DUlH:-##L2_V-$[5N#,L]rHKrk;H$),##(5>##,AP##0Mc##4Yu##:l:$#>xL$#B.`$#"
    "F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#c9F&#gEX&#kQk&#o^''#sj9'#VpB'##'U'#'3h'#+?$(#/K6(#3WH(#7dZ(#;pm(#?&*)#@2,T'ZL]w'<#=nNK/###+)l-$^do-$02q-$"
    ",*+##bsj-$rJp-$06+##M4m-$/I6?$?G:;$%ZlS..RUV$;>pV-6s2t_<I@&c5]AmABd4R*QGm92m9x%#lFRqLft,wL>Hbr.^g*##Gfl-$.V>s%7'as%x*35&HTW8&8$W8&LaW8&Mga8&"
    "QdPhL$d;0([RXgL*`ta**?ga<r'/dW4@[<$99.&G02O2_=9>n&b'4kO6s-(#4_A5#32bjLPQ[I-RdF?-on8Z.q-%##;(m<-f;p_/Lj&##>R%v#Z]s&#R<&5S4wV<%-A###)EsI38c1p."
    "MLZ>#:^o0#]/G>#'$Gj>g5dI#e->>#h9,ip5xcrQElUw02Eaa4Jk_a4;a#+%`:j=.CD)H*k?.lLG3gg1p_XF-^+Sc,C@@#-nugiL77Z<-'mr*N#V;;$,heHaV@VS%NA<A+F7$##-a55M"
    "uF`hL*gYs-`,oeM)gUN(oD&$2?PYk,=qc#1KGvs04VqFM+i%6M`(TH#]%@L,.8st(3T^[MK;8F-#YlS.$+AP#Ig=(.EL2.NLq1q>JatOYd&9DEa1BCY&:Z0NEqiR#C'4vLHeQLMel,>%"
    "MfYs-BCl(N>5aINpj7p,?7%WMjx/W;F9/tRc9hw'&/PxL3dlHM;<JYMCXFRMa1$##[W5#Zxjd9D*lYg)5q3W-3lFRNNrm$TDK#&Y0.sa44c+E*J>+E*`vk,Nm9BP#*]^C-+*qH%<Puw9"
    "e=WU2A5l;-U5?l.-Mc##[))q%rIn8/ITM&.JL5gLbRo8%?xAx$tdJe$0VKF*H@'_,oF>c4:*wD4>eJe$/_#V/.4$_>bd,5LTta:.Hq2[,MMC;._QdP1(f7_#.UZw5<NS*NZB6(1i=vN+"
    "M7oi1vPK_,Em1P0$Buk0#bxj0NgxA,@&r$'=Abo7_4)dax:*,)BVq)4m>:Z-qdWb.Lov[--Eh531<8DE9A5>#M60q%:$,3'(/n3MR0XBMJq-F>hI:;$dkjOo=R%5S&v2b+fh18.G/IL("
    "ODaEe_kY)4B8T;-haCL.)w@+4<**=(73k)4$OP`0<O`A>u;^;-*8@m/eoZoR$2AYY^H)=-5l@u-PiRoLr`U`Nk2fp&;l###vxPA#i1ur-p;>x6jH3Q/`c*E*%l=V/@3MY5ZIASBBZY8."
    "dBib,$PkA#+vYhLQ9?LO2rJfLwEPGM7Nu$MoV&%#*bO3b;c>K)[(Ls-b3NZ6CE(E#V)ZA#5Qe<CRcoD*eAI&4V1@`a@j(N'dvv)NHo#(LdMF1pO)RM0mv?t$=EE5A)AQXNks&nLGfD^:"
    "qko.QBv#OFv.=GM6%axLl`6##4,>>#b&hv-;kQq7l8CW-<KLB#N-;(s$SFi(Oro'+pI=VHXm/M(Lql'+N],##TC?dF-`6^6O_R%#1XG12*7pL(,39Z-]p*P(+ot'4awl)4'pg9;VlK/Z"
    "$58wu0$(*+cp2##Li_3=<6x,vvVR:0M=][u%V`B>am$/?%)pu,wPZ$6:Ad,2ac%H)+,Bf3ZSo#/6@$E>5jpW_E[Z(+hch2Le@YHENqIu-7]JfLw?,gLRX?>#PXF.#+GY##BGOvI28R]4"
    "sP/J3GKi-M6^b?/L_G)4TqdLM@P?14DG2G>`9s#MvOFnMgQ5d0$Qd0BP,8>#Qj.iL:=PS7M/B+i#%O'#9pg<C7cB$$5FNP&Q]8>,5wnU.qH7g)#5T;-)x:T.o`0i)u$h`$[7E_&=)k.3"
    "^nr?#1,6*4Hg2uR$)'fh[ta3Lx7qW7c4ue)>I[W$WGFtCaLtQ0NTiB#8gN-*;6FiaZ[S7&9%4E&p*v,*VGFFNKV1U#$M7%#H$Oa%`10;6iH%##4;Ls-j(]]4I>WD#6*YA#ubLs-7:E:."
    "__OF3)Y:p7Y9Z;%[v]G3L2R_YMo5,*C%x:/DTk%$#2&p&-8^6NVZ56^JHGN'950kLhAU6&,pl5&x7@@'X]SIE>Q.l'uT:H3u'^-%>@UT7<u..MXRGgL/*:q%0>ClLD5K;-&;<Y.:A###"
    "EY:a%CX;p7p='$-$Z:D3$IDR8Q0G)48+pGM^b$q$iF?lL.+.m/q9k=._&B.*u;ZE&G*[V0s&9;-X8Vp.QbTY>tWH&mbT7DE<Ju]>n_4?-1s8gL&C+h>sO#T%GkG&#tUB#$1PId*kLGM9"
    "2g(I3'F/[#4N1)3qX5l10Kjb$/+^H<1eR>6R%/+%uU`i0p[6:OB_rX-LY(R<jGH`akkZV$.D###ZHSP/$Wk-$nk=V/:iqH.ftnMU?<;HE9=G)SDmv8Md)rtL.tiR#s=+?._8),)8KkL#"
    "_qt.L.NO.$I+%X7;+)kLf^Lh.Q9SP#o&%`%&e_c)DjLO'u*^=%3H()3KJjD#okOF3sW(PoP/.9S>xf[@JX?uY<C$q8XGqG2FJ@t1%'Ph#rrQTYJeVkL;`#`-ajd<CWrC$#`QA7/f'G[$"
    "qYDD3n0CG)m)<.3HpEG#jWjgLMLN=.V2^t1tlIfLsns.L^A4_]K%5H327gfL*6ki0$.DP>$(OM0$/UM0Nbk-$6ws4S$0]Y#-GdN(GaAW#6tI0S:ZUj0X4=&#SD6C#kmqc)hkMH3;O'>."
    "Ejb59n/,H340_:%u4_sISdK?7MxYB]W-Os'tN-90q+Mv-uuIi1h0AN#SL2D+7G,jL>LR*5+BJ#60X%K)KKm2r,hLU#$<r$#)VC_%BCs?#lJv;%3P5c4[HuD#+K;<%j,LB#$qYj^90$R0"
    "d].C-GWWP&F9Yk0)hq1TRX*j:N%5n0U]>'(6%lq&<O=^0vC%6/&05##`FFgL?si-2jn0N(:KLB#&H]#1%D:>>;w$sR/u4F%p1i/)L*UfLRO1N(q5;v16bT<_t3wK#@/[]-mi]&?a^>V/"
    "%W?n0O%hg7$&5>#:WLpKQpF?-]39oLHjCw0i/CW.%8YY#t=58.hMUY>tPtNX:&jNXF9;T0Xe)i-s`$UV@hC#$q@]ihSss8&QlRfLv5S8%+Ng*%NDj_7Q?(joTr9jMLNG)SR3H&#;m]ca"
    "S0Q&#[<eE.`kxIE<kv-?f7J.?L*nO(x&_:%$Gp;-<C;>8qZN-v;-KZ-,is.L<kYhL>wHA/[oj1#EBFl9Rj^o0jfE.34+gm0jH3Q/,xr?#:?(B#$kl;/0AG2;]>l(5%/[a,q=<88L]rB#"
    "UmvM;A?J'5bkXa,PSrt-jQ,GM$*0fhR<&5S?=KV6B^u-*-<Tv-IV]C4TH^R/1j$@'6/TF*.p+c4'&'f)0Zc8/HjE.3x8w)4dC10E@WI]OdD24'2Q[@%EtD+*cRF=.JliEI_B`C$v.]h("
    "8$X)<SD,B,Ir*g(@Q_5%bFU_+>gW=&MT.C#<iVU8MJf[>uK@%%#N<n1M6OD<Xng,3<#$9%uU6^9M#h>$uFPS.gH#/LF9Y[$YP###;:k=./5T;-:c1p.+w@+4<9t0#K4..$H'mTM;ZT`9"
    "H(7W]EHjj9)LF&#dFD<-81kB-)YHj9)nF$.N$H=9I3_B#xu+cufY_[>Fr/gL[S5L%i*GJ(D1$##&`p>,o]wE74#Mc%hO1x5u'7]6L77<.xj(P0PLt'+U17I$-<JwYxI+M(9l/M(PU]B>"
    "O;[ahZu]GMm4##)eXRb%uWsm'U]W]+de+wpWYJ#$/3kM(Uu&<.>'5x#9jt;-+fG<-VE0W$7@[5L&gm1ZN)5G>%XrIh-K-F%N_^HFOKZ:Z$,G3LWgSe$-sOA>xOF&#>k]&?\?E`hL]+0i)"
    "&_j-$39k-$hcO/M_DsI3GH:e?rVi.LW&tn0N%U`0%-eK>OUvD>QeSTo8('_-Pm9LN]9W$#E58Z25HSP/]Mx_4bv)E*bPD<%vlkO%k%@W.x2doL,(;T.1soQ0FAm<-8(xU.4^%_>gHf48"
    "#AMd*4Ar%4F^?k)*YrB#]AqB#TekD#N4Bk$<mQv$-5gF4^IcI)?@i?#EkWN0$p%&4D)Jd)(Gbr9KCi/:Y0?T%D;`Z>'$qba:nWM0M6^Q&Rgl3'kBYj'fT:S09l/M(Nuo'+8%0q%::8@#"
    "B5:i:eEkp%S0+X$UZTq%Q,*E*CG5`OP7/$4<jqR#9N7%#n,_'#vCEj1SBo8%1?Ha&n77<.A#=#.fE$>%1F$`4P#G:.*dD<%6$;g?\?MhZ-Y)s'+YIlP0:5ecXV2%h(K4Wu1AEO(5fKCH<"
    "AqBE#<s5c*a*sZu5Hv&5[sP3'TW1xNUiF]-g`Wd6';G##O40U#KLE73L?O&#rpB'#962N(7%x[-XEeq7S$wY679Bk04tFA#jAqB#V#_#$b55W-9wc?KF,]:Z@FvX$-7ji0HiZ%?xee,;"
    "P70ALOro'+ZD>;'Ak,M(SDeIMKS]@%?9YHtPbVV$(DNfLIQV'#9@%%#bd0'#(3.^$6nNINo?sI3Tia;.L^87.^'q+Mfi=4%OIHg)P[IMh:X[5LO?9q%%FvJE=*$RE='=%&Gg`hL#/Bfh"
    "#0ki0n.Rk+sNEEEGF=C&2-2tLDH[U7U8:T.<jqR#xXs#0argo.VRG)4fON?'*cQP/O<IM9m(c/EaK1-6H1`,1#E80u?N09.aC`L;vS*^#aC'HEgK[Y#^(#C#&c+E*rC-DEjuuqLF<jX0"
    "'UMfLo`?uu&:SP#`5i$#Qe[%#2`SBfw,4R8sNu)4k9k=.)#bWq=hwea$B+M(:b^+*f/An0%^rB#8bU^uK87uLX3[kalI#3MfH<mLklv##fJ]]%tA2<%J>`:%J0fX-a`0i)@H7g)o9EGE"
    "MC-buZK7[ugc.U#>shw'B@Ew0MiWHE&i@m/$&>uuG$6V#M)KZ.tNHg)#(B?.J>+E*U^Bb4'>-##eJl.-e/g`Oi=cD3fU4E#]G^7=%/p`4r'g%-sDreXWI,#v=Gt$Mr9MhLFY,&#jrgo."
    "qc4c4FRR1MGMB.*a9OA#]Z-H)QNv)4qxN$%fQLnu9_<22S%].2PLNe$o4oQ0OFC+*OITI<c+g'+cT5N'%lfK>,CP,ETsF*[u9&T.g2e0#<SbS4aKb&#P,>>#kp-)*hc[:/wkXI)fK.29"
    "OGgv-%NCD3.cUp$Ml%$$X)^,3p$UfLs8:K2%%?03R5Y*+c4q0#JvCl0NN3G>CqL_&-V@79&)*4:;44bI&Ng8.$VcB>'`O]ub%Os7&jYca#DCda/(no%43CwKH`x_485PdNK:-##L))i%"
    "OCwH.fwMq/6GTp<t&IrRf^Gs-n`(JUV*elLn%Ka#,VC;$0o$s$41[S%8I<5&<bsl&@$TM'D<5/(HTlf(LmLG)P/.)*TGe`*X`EA+]x&#,a:^Y,eR>;-ikur-/X&97od9B#spKB#w&_B#"
    "%3qB#)?-C#-K?C#1WQC#5ddC#9pvC#=&3D#A2ED#Ico]$t(2eGu:`dGT.+W1pC=r(<UbmBxdM=BE-V6M@B.eM2<*hFUDk?Ho)bNEDq0NC1NKKFVtb;.%,rEHP8lM(q@,^G86n3N=6rdM"
    ";>5/G`lT&$lDq$&dZ7uB$krTCnJbQDhg[UCtWi=BjpojBOm2bIE%VeGe8Zw&og`TCTNOGHx5xUCdL^kEs09I$MH*hF$ToUC?j4HMF5x0FkIg0+.Sv1F=d;eEL(fV1)8$lErm`9CYQ]/G"
    "A4i4+Tb;2FnAFVCp[DhFw#fUC<%vLFQ^@rLDqIG$v)/:C6V3oMS^WhFf/J=B=ln+HFKbk3%]%iF&<$@&8GhoD<00iFsi;#G88;X-=n)F.@%CVC*DM*Hn%luB,WX;C0gkVC,uDPE7R+RB"
    "v_ECIb4>@'8]k$'2sq$'=e*_ImjVm07`I'I:F4,H(xXVCdY5m'bZwM1JAnw'p+qP'>&w5/O8-vG./jx&0d,hFE.ddG&PeFHoiDtBQfRV1:bk(Irh)pD[<X0MejEN061=GHf@9oD>HnrL"
    "6)h>$6ZxUCiL^kEGpcfG^mJ3B1u3GHB7VEHk#/>Bt@vgF8T(5/V#DcHX6_t1(x#kE$^pgF#xLN1>x7+HE_WI-aA8$gb`ubH3Gv-GgG6&FhAD='9Y</DJ[.<-$QN32[A7DIsqx;H(WKSD"
    ".&KKF`.ubFrGv@&/or*HAURfGw>BNB98rU1oF]>-_'Z<UK&;qL:#nKF?06kC4x3,HUBffGvS2='5I]20K=_oDfwi-##m8Atin2#`";
