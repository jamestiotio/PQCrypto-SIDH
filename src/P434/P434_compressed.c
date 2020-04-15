/********************************************************************************************
* Supersingular Isogeny Key Encapsulation Library
*
* Abstract: supersingular isogeny parameters and generation of functions for P434_compressed
*********************************************************************************************/

#include "P434_compressed_api.h" 
#define COMPRESS
#include "P434_internal.h"


// Encoding of field elements, elements over Z_order, elements over GF(p^2) and elliptic curve points:
// --------------------------------------------------------------------------------------------------
// Elements over GF(p) and Z_order are encoded with the least significant octet (and digit) located at the leftmost position (i.e., little endian format). 
// Elements (a+b*i) over GF(p^2), where a and b are defined over GF(p), are encoded as {a, b}, with a in the least significant position.
// Elliptic curve points P = (x,y) are encoded as {x, y}, with x in the least significant position. 
// Internally, the number of digits used to represent all these elements is obtained by approximating the number of bits to the immediately greater multiple of 32.
// For example, a 434-bit field element is represented with Ceil(434 / 64) = 7 64-bit digits or Ceil(434 / 32) = 14 32-bit digits.

//
// Curve isogeny system "SIDHp434". Base curve: Montgomery curve By^2 = Cx^3 + Ax^2 + Cx defined over GF(p434^2), where A=6, B=1, C=1 and p434 = 2^216*3^137-1
//
         
const uint64_t p434[NWORDS64_FIELD]              = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFDC1767AE2FFFFFF, 
                                                     0x7BC65C783158AEA3, 0x6CFC5FD681C52056, 0x0002341F27177344 };
const uint64_t p434p1[NWORDS64_FIELD]            = { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFDC1767AE3000000,
                                                     0x7BC65C783158AEA3, 0x6CFC5FD681C52056, 0x0002341F27177344 };
const uint64_t p434x2[NWORDS64_FIELD]            = { 0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFB82ECF5C5FFFFFF,
                                                     0xF78CB8F062B15D47, 0xD9F8BFAD038A40AC, 0x0004683E4E2EE688 }; 
// Order of Alice's subgroup
const uint64_t Alice_order[NWORDS64_ORDER]       = { 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000001000000 }; 
// Order of Bob's subgroup
const uint64_t Bob_order[NWORDS64_ORDER]         = { 0x58AEA3FDC1767AE3, 0xC520567BC65C7831, 0x1773446CFC5FD681, 0x0000000002341F27 };

/* Basis for Alice on A = 6, expressed in Montgomery representation */
const uint64_t A_gen[6*NWORDS64_FIELD]           = { 0x6E18D3A63313A738, 0x1DCC496DD6DDE298, 0xA35F3F7DAFBE2B43, 0xC6B9A5CC670071EB, 
                                                     0x2EA3DB085283675A, 0xFDFE173A0297F36,  0x0002200804EB824D, // XPA0
                                                     0xB999E9E259F7BFA8, 0x2584D67D0C2EEAA9, 0x80AB07D4E9625724, 0x781DA616A7A76E54, 
                                                     0x9BE449736374F491, 0x8C6F86E8B0C4D74A, 0x0001C1D4812CBD98, // XPA1
                                                     0x257DBD53095FD263, 0xBBB3C7A7B4EDB1D4, 0xA817B7FDDD5BB8DA, 0xF5DE963B242B7AB3, 
                                                     0x7F51B5362FC94CB6, 0xE7D2496B526DFF16, 0x0001E962CF69118C, // XQA0
                                                     0xED9DC89467FB039D, 0x17C71E114B5803D0, 0x816C3379BE9647BF, 0xB07F441A15434B64, 
                                                     0xCC65C1804AF4CBD1, 0xF06BF5F074032C77, 0x0001A251F94CF02C, // XQA1
                                                     0xA26194AB4BD1A16F, 0xCFCD9F7F04D5AB10, 0x1BB4A7C04C37482C, 0x71DEE733632DA36D, 
                                                     0x7335784B5ECF957F, 0x66AE2381533A7F09, 0x000232BFFE6FA42F, // XRA0
                                                     0x60ACBE5D899CFA6A, 0x82AC55A556E5A22F, 0x437D8C2AC83FDC6B, 0x620A8DA602543EDE, 
                                                     0xD19ABA8092A1E8C2, 0xAFF1AA61981C95D3, 0x0001A7232B0C035E }; // XRA1

/* Basis for Bob on A = 6, expressed in Montgomery representation */
const uint64_t B_gen[6*NWORDS64_FIELD]           = { 0xE172658571249BA8, 0x9D8F52CB15829DA0, 0xE3A7C7F9F0E3F832, 0x8B825DD0B9410D30, 
                                                     0xF42F815734752EDA, 0xCB35DD9160997586, 0x00018B3AAAAD0F79, // XPB0
                                                     0xCF0B435C40C1375D, 0x58AC8A63992B36EF, 0x416D0B3DFB0C1DF5, 0xB257E9CFE8985F15, 
                                                     0xA493D98A7A1D6DF2, 0x6D6781A5B3FDE61F, 0x000179AC0D886A3F, // XPB1
                                                     0xE172658571249BA8, 0x9D8F52CB15829DA0, 0xE3A7C7F9F0E3F832, 0x8B825DD0B9410D30, 
                                                     0xF42F815734752EDA, 0xCB35DD9160997586, 0x00018B3AAAAD0F79, // XQB0
                                                     0x30F4BCA3BF3EC8A2, 0xA753759C66D4C910, 0xBE92F4C204F3E20A, 0x4B698CAAFA67A0EA, 
                                                     0xD73282EDB73B40B1, 0xFF94DE30CDC73A36, 0x0000BA73198F0904, // XQB1
                                                     0x9F7367022EFDF650, 0xA8C21C687A91D6BC, 0xDDB909C497C4BFED, 0x66FD362A30232EBF, 
                                                     0x84AC5026408590E1, 0x5378004CB74DA4ED, 0x00008AA46B9E55B2, // XRB0
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }; // XRB1

/* The x-coordinate of lB^(eB-1)*QB, expressed in Montgomery representation */
const uint64_t XQB3[2*NWORDS64_FIELD]            = { 0x821E631A922B742C, 0xDA3EB61A6B066600, 0xBE9BF948971BD406, 0xAEF5D42FAF04075A, 
                                                     0x70165C6A99378F7F, 0xEEE1058485DD1F9A, 0x1B20B9F7D4D95,
                                                     0x787CA8B336944EF3, 0xD29EBA9AF11A6EB, 0xE0BC6DA29BB99844, 0xF583F3C8DA2FB5FA, 
                                                     0xBBCB2EAAD861B7E8, 0x2487A88C3E782D0A, 0x11BA37E3442A5 };

/* Basis for Alice on A = 0, expressed in Montgomery representation */
const uint64_t A_basis_zero[8 * NWORDS64_FIELD] =  { 0x6E18D3A633148F91, 0x1DCC496DD6DDE298, 0xA35F3F7DAFBE2B43, 0x3B18175B7C0071EB, 
                                                     0x62E0C886CC6A1260, 0x75482A45DD52E0F9, 0x1C5C62D4E6CBE,
                                                     0xB999E9E259F7BFA8, 0x2584D67D0C2EEAA9, 0x80AB07D4E9625724, 0x781DA616A7A76E54, 
                                                     0x9BE449736374F491, 0x8C6F86E8B0C4D74A, 0x1C1D4812CBD98,
                                                     0x9B989BE60CFF0D15, 0x8B80A32171813F53, 0xF4F067606A56228E, 0x48F8237E159577B0, 
                                                     0x42529574B9E74156, 0xD8D26313F4AA9F9C, 0x1279AC6BC876C,
                                                     0x9597544CBE9D88DF, 0x13801F440DF32748, 0xE4ECAFF9C15D0CEB, 0x7867D92EB045A646, 
                                                     0x2399062BA8C64EF, 0xE9258C0BDF8BBFF7, 0x1CE4BBF872205,
                                                     0x257DBD530960BABC, 0xBBB3C7A7B4EDB1D4, 0xA817B7FDDD5BB8DA, 0x6A3D07CA392B7AB3, 
                                                     0xB38EA2B4A9AFF7BC, 0x4D3A923D8F9760D9, 0x18F20F7CBFBFE,
                                                     0xED9DC89467FB039D, 0x17C71E114B5803D0, 0x816C3379BE9647BF, 0xB07F441A15434B64, 
                                                     0xCC65C1804AF4CBD1, 0xF06BF5F074032C77, 0x1A251F94CF02C,
                                                     0x6239F7D1AD27CE42, 0x717E2E83920EF08C, 0xAA9328D01C5FDF24, 0x824E9DEA2B1F02F9, 
                                                     0x20B1D0984E195350, 0x4A5AFBFD02EF9414, 0x228BCA648BAFF,
                                                     0x7DEDAD20B53F2C9A, 0xE7DD8746364EACC1, 0x743BABC72A5096D4, 0xDDA2FBF4E96A5174, 
                                                     0xE05A5B3B71083AF0, 0x69AB2A817C72ADCC, 0x216CFFC723E3C };

/* Basis for Bob on A = 0, expressed in Montgomery representation */
const uint64_t B_basis_zero[8*NWORDS64_FIELD]    = { 0x214C34BB192F67A0, 0xDD49D3D02115D30, 0x700652C1A7B66ED, 0x1F856B48F4FF0024, 
                                                     0xFBDE6F4E6A705221, 0xB951A3D6C93D87B8, 0xAE8ADB818ED6,
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                                     0x51D889FE197209C1, 0x191BCD9DBE4FE0EF, 0x447818CF5E54DD8A, 0x3F42710E8562A583, 
                                                     0x647BDBB01C66DCB5, 0xF402D36C15EA12E1, 0xA1E1D287C14C,
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                                     0xDEB3CB44E6D0985F, 0xF22B62C2FDEEA2CF, 0xF8FF9AD3E5849912, 0xDE3C0B31EE00FFDB, 
                                                     0x7FE7ED29C6E85C82, 0xB3AABBFFB887989D, 0x185944B95E46D,
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                                     0x51D889FE197209C1, 0x191BCD9DBE4FE0EF, 0x447818CF5E54DD8A, 0x3F42710E8562A583, 
                                                     0x647BDBB01C66DCB5, 0xF402D36C15EA12E1, 0xA1E1D287C14C };

/* Full 3-torsion for Bob on A = 0, expressed in Montgomery representation */
const uint64_t B_gen_3_tors[16*NWORDS64_FIELD]   = { 0xD697601DCA7CA4B5, 0xD16726DCBE0FD988, 0x8119DD7AF0E6C87C, 0xD1E1BDAB620C8DF9, 
                                                     0x27ABDBB336AF35D0, 0xC36900B91B5F4914, 0x1E21CCC021AE9, // XPB30
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // XPB31 
                                                     0xDAEB901A8B0B1BF6, 0xCFA413FEE3DEEB, 0xD8A152FC022EDD7F, 0x69AEE6F393ADDBE5, 
                                                     0x45F3B54D85AB6DDE, 0x19F7181A0B697BAB, 0xE1C0ED0125C4, // YPB30 
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // YPB31 
                                                     0x29689FE235835B4A, 0x2E98D92341F02677, 0x7EE622850F193783, 0x2BDFB8CF80F37206, 
                                                     0x541A80C4FAA978D3, 0xA9935F1D6665D742, 0x52025B15585A, // XQB30 
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // XQB31 
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // YQB30 
                                                     0xDAEB901A8B0B1BF6, 0xCFA413FEE3DEEB, 0xD8A152FC022EDD7F, 0x69AEE6F393ADDBE5, 
                                                     0x45F3B54D85AB6DDE, 0x19F7181A0B697BAB, 0xE1C0ED0125C4, // YQB31 
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // XRB30 
                                                     0xFC42C90C960A3D15, 0x815F603FE9413385, 0x640A06136E6DF1BA, 0x71E4C95A57555FE7, 
                                                     0xBF7BAC5BE8654D6F, 0x27CD8B340B46A743, 0x259C6CF43995, // XRB31 
                                                     0xFF585E4DCB517F6A, 0xEFFCE64BA91FBE1C, 0xFAEB260583830D6E, 0xEB68E9A0F32E8E96, 
                                                     0x419A555BABEC214B, 0xBA7A5BFD1EA7D14F, 0x220892411AA66, // YRB30 
                                                     0xA7A1B234AE8095, 0x100319B456E041E3, 0x514D9FA7C7CF291, 0x12588CD9EFD17169, 
                                                     0x3A2C071C856C8D58, 0xB28203D9631D4F07, 0x13960305C8DD, // YRB31 
                                                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // XSB30 
                                                     0x3BD36F369F5C2EA, 0x7EA09FC016BECC7A, 0x9BF5F9EC91920E45, 0x8BDCAD208BAAA018, 
                                                     0xBC4AB01C48F36134, 0x452ED4A2767E7912, 0x20E82BA2339AF, // XSB31
                                                     0xFF585E4DCB517F6A, 0xEFFCE64BA91FBE1C, 0xFAEB260583830D6E, 0xEB68E9A0F32E8E96, 
                                                     0x419A555BABEC214B, 0xBA7A5BFD1EA7D14F, 0x220892411AA66, // YSB30
                                                     0xFF585E4DCB517F6A, 0xEFFCE64BA91FBE1C, 0xFAEB260583830D6E, 0xEB68E9A0F32E8E96, 
                                                     0x419A555BABEC214B, 0xBA7A5BFD1EA7D14F, 0x220892411AA66 }; // YSB31

/* Pre-computed pairing ReducedTatePairing(R0,S3,lB^eB) on A = 0 */
const uint64_t g_R_S_im[NWORDS64_FIELD]          = { 0x410F318D49162E42, 0x6D1F5B0D35833300, 0x5F4DFCA44B8DEA03, 0x908ADE1CD38203AD, 
                                                     0x100CD330A23B7494, 0xE0A2D716A265D0DA, 0x1C5F4777BD5A5 };

/* Pre-computed pairing ReducedTatePairing(phiR,phiS,lB^eB) on A = 0 */
const uint64_t g_phiR_phiS_re[NWORDS64_FIELD]    = { 0xE3F6DD5BAE3DA160, 0x893B9874EAD27B9A, 0xAEB72FD8BCC2583, 0x2FFCCC8FAFA395E, 
                                                     0x2303AE01DD252409, 0xC6AD33482DFD53FA, 0x978AD00D3221 };
const uint64_t g_phiR_phiS_im[NWORDS64_FIELD]    = { 0xC728DE8E32AF6622, 0xF179EECB0D4F3D28, 0x37CA8F90B93772D7, 0xA349AE99A7B40196, 
                                                     0xD81D98E9D4891BF8, 0x4767F6BC20C22291, 0x19AF8C13C99B8 };

// Montgomery constant Montgomery_R2 = (2^448)^2 mod p434
const uint64_t Montgomery_R2[NWORDS64_FIELD]     = { 0x28E55B65DCD69B30, 0xACEC7367768798C2, 0xAB27973F8311688D, 0x175CC6AF8D6C7C0B,
                                                     0xABCD92BF2DDE347E, 0x69E16A61C7686D9A, 0x000025A89BCDD12A };                

// constant Montgomery_RB1 = (2^NBITS_ORDER)^2 mod Bob_order
const uint64_t Montgomery_RB1[NWORDS64_FIELD]     = { 0xE63F0179FFC3EF1B,0x47AF4CC2440BEB81,0xEC3CD079857407E7,0x8BF47C};


// constant Montgomery_RB2 = -(3^OBOB_EXP)^-1 mod 2^NBITS_ORDER
const uint64_t Montgomery_RB2[NWORDS64_FIELD]     = { 0x7A9991106B9F6535,0x7E06A4ACAEA6DA73,0x3D4C8710FC0ECD0E,0xAE3D0E8DC11F55F1};


// Value one in Montgomery representation 
const uint64_t Montgomery_one[NWORDS64_FIELD]    = { 0x000000000000742C, 0x0000000000000000, 0x0000000000000000, 0xB90FF404FC000000, 
                                                     0xD801A4FB559FACD4, 0xE93254545F77410C, 0x0000ECEEA7BD2EDA };

// 1/3 mod p
const uint64_t threeinv[NWORDS64_FIELD] = {0x5555555555557C0E,0x5555555555555555,0x5555555555555555,0x3C30F5A8EB555555,0x9A84C9F93D7058B4,0x410E5C007655D5E8,0x1C70EFCA40721};

// Fixed parameters for isogeny tree computation
const unsigned int strat_Alice[MAX_Alice-1] = { 
48, 28, 16, 8, 4, 2, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 8, 4, 2, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 13, 7, 4, 2, 1, 1, 2, 1, 1, 3, 2, 1, 1, 
1, 1, 5, 4, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 21, 12, 7, 4, 2, 1, 1, 2, 1, 1, 3, 2, 1, 1, 1, 1, 5, 3, 2, 1, 1, 1, 1, 2, 1, 1, 1, 9, 5, 3, 2, 1, 1, 
1, 1, 2, 1, 1, 1, 4, 2, 1, 1, 1, 2, 1, 1 };

const unsigned int strat_Bob[MAX_Bob-1] = { 
66, 33, 17, 9, 5, 3, 2, 1, 1, 1, 1, 2, 1, 1, 1, 4, 2, 1, 1, 1, 2, 1, 1, 8, 4, 2, 1, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 16, 8, 4, 2, 1, 1, 1, 
2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 8, 4, 2, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 32, 16, 8, 4, 3, 1, 1, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 8, 4, 2, 
1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 16, 8, 4, 2, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1, 8, 4, 2, 1, 1, 2, 1, 1, 4, 2, 1, 1, 2, 1, 1 };

// Fixed traversal strategies for Pohlig-Hellman discrete logs
const unsigned int ph2_path[PLEN_2] = { // w_2 = 4
0, 0, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 9, 9, 9, 10, 11, 12, 13, 13, 13, 14, 14, 15, 16, 17, 18, 19, 19, 19, 19, 20, 21, 22, 22, 23, 24, 25, 26, 
27, 27, 28, 28, 28, 28, 28, 29, 30, 31, 32, 33, 34, 34, 35
};

const unsigned int ph3_path[PLEN_3] = {
#if W_3 == 4
0, 0, 1, 2, 3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 13, 14, 14, 14, 15, 16, 17, 18, 19, 19, 19, 19, 20, 21, 22, 23, 24, 25
#elif W_3 == 5        
0, 0, 1, 2, 3, 4, 5, 5, 6, 6, 7, 8, 8, 9, 10, 11, 11, 12, 13, 14, 15, 15, 16, 17, 18, 19, 20, 20, 20
#endif        
};

// Entangled bases related static tables and parameters

// Constants u and u0 where u = u0^2 in F_{p^2} \ F_p used in entagled basis generation
// For the 2^eA-torsion basis generation:
//      Two tables of 17 elements each for the values r in F_p such that v = 1/(1+ur^2) where r is already converted to Montgomery representation
//      Also, 2 tables for the quadratic residues (qr) and quadratric non residues (qnr) v in F_{p^2} with 17 GF(p^2) elements each. 
// For the 3^eB-torsion basis generation:
//      A table of size 20 for values v = 1/(1+U*r^2)

const uint64_t u_entang[2*NWORDS64_FIELD] = {
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xE858,0x0,0x0,0x721FE809F8000000,0xB00349F6AB3F59A9,0xD264A8A8BEEE8219,0x1D9DD4F7A5DB5};

const uint64_t u0_entang[2*NWORDS64_FIELD] = {
0x742C,0x0,0x0,0xB90FF404FC000000,0xD801A4FB559FACD4,0xE93254545F77410C,0xECEEA7BD2EDA,0x742C,0x0,0x0,0xB90FF404FC000000,0xD801A4FB559FACD4,0xE93254545F77410C,0xECEEA7BD2EDA};

// Tables for quadratic residues and quadratic non residues v with 17 elements each. 

const uint64_t table_r_qr[TABLE_R_LEN][NWORDS64_FIELD] = 
{{0xE858,0x0,0x0,0x721FE809F8000000,0xB00349F6AB3F59A9,0xD264A8A8BEEE8219,0x1D9DD4F7A5DB5},
{0x244DE,0x0,0x0,0xA1CCD72326000000,0x407B7FF8496D02DF,0xB402E5F8D9CA0493,0x386AF88303BD},
{0x2B90A,0x0,0x0,0x5ADCCB2822000000,0x187D24F39F0CAFB4,0x9D353A4D394145A0,0x12559A0403298},
{0x3A163,0x0,0x0,0xCF3B3CB737000000,0x4CBA127218F35AB9,0x29D831F766AA763,0xCB17C8A31D0A},
{0x4158F,0x0,0x0,0x884B30BC33000000,0x24BBB76D6E93078E,0xEBCFD773D5E1E870,0x1B80670604BE4},
{0x5E641,0x0,0x0,0x710813DA5D000000,0x8D35926A62605D99,0xB6A069185034ABF6,0x10382C12620C7},
{0x7B6F3,0x0,0x0,0x59C4F6F887000000,0xF5AF6D67562DB3A4,0x8170FABCCA876F7C,0x4EFF11EBF5AA},
{0x82B1F,0x0,0x0,0x12D4EAFD83000000,0xCDB11262ABCD6079,0x6AA34F1129FEB089,0x13BEDB9A92485},
{0x89F4B,0x0,0x0,0xCBE4DF027F000000,0xA5B2B75E016D0D4D,0x53D5A3658975F196,0x228DC61665360},
{0x91378,0x0,0x0,0x87335C8C98000000,0x1EDFFE125B40B7E,0xD00B97E36728124D,0xE1ABE20C0EF6},
{0x987A4,0x0,0x0,0x4043509194000000,0xD9EFA4DC7B53B853,0xB93DEC37C69F5359,0x1CE9A89C93DD1},
{0xAE42A,0x0,0x0,0x6FF03FAAC2000000,0x6A67DADE19816189,0x9ADC2987E17AD5D3,0x2D2832D1E3D9},
{0xBCC82,0x0,0x0,0xE21027B4BA000000,0x1A6B24D4C4C0BB32,0x6D40D230A06957ED,0x20705824C418F},
{0xCB4DB,0x0,0x0,0x566E9943CF000000,0x4EA812533EA76638,0xD2A91B02DD92B9B0,0x1ACC3AAAF2C00},
{0x10563F,0x0,0x0,0x27E85F8023000000,0x1F9BC84D2642124E,0x684A3E4BD23840BD,0x43BC4C3AD5C6},
{0x10CA6B,0x0,0x0,0xE0F853851F000000,0xF79D6D487BE1BF22,0x517C92A031AF81C9,0x130AAF3F804A1},
{0x113E97,0x0,0x0,0x9A08478A1B000000,0xCF9F1243D1816BF7,0x3AAEE6F49126C2D6,0x21D999BB5337C}};

const uint64_t table_r_qnr[TABLE_R_LEN][NWORDS64_FIELD] = 
{{0x742C,0x0,0x0,0xB90FF404FC000000,0xD801A4FB559FACD4,0xE93254545F77410C,0xECEEA7BD2EDA},
{0x15C85,0x0,0x0,0x2D6E659411000000,0xC3E9279CF8657DA,0x4E9A9D269CA0A2D0,0x92ACD020194C},
{0x1D0B1,0x0,0x0,0xE67E59990D000000,0xE4403775252604AE,0x37CCF17AFC17E3DC,0x17F9B77DD4827},
{0x32D36,0x0,0x0,0x13ECBF2D1E000000,0xF07EC9EEF4AC5C89,0x86678EA198B886AC,0x2124847FD6173},
{0x489BC,0x0,0x0,0x4399AE464C000000,0x80F6FFF092DA05BF,0x6805CBF1B3940926,0x70D5F106077B},
{0x4FDE8,0x0,0x0,0xFCA9A24B48000000,0x58F8A4EBE879B293,0x51382046130B4A33,0x15DC498C33656},
{0x57215,0x0,0x0,0xB7F81FD561000000,0xB533ED6F0CC0B0C4,0xCD6E14C3F0BD6AE9,0x16941968F1EC},
{0x65A6D,0x0,0x0,0x2A1807DF59000000,0x65373765B8000A6E,0x9FD2BD6CAFABED03,0x1F07168E34FA2},
{0x6CE9A,0x0,0x0,0xE566856972000000,0xC1727FE8DC47089E,0x1C08B1EA8D5E0DB9,0xA940E9890B39},
{0x742C6,0x0,0x0,0x9E76796E6E000000,0x997424E431E6B573,0x53B063EECD54EC6,0x1962F91463A14},
{0x9FBD1,0x0,0x0,0xFB91CE1BAD000000,0x362AED5F9F9AB683,0x3573E0B5A4517410,0x876A0A6EF968},
{0xA6FFD,0x0,0x0,0xB4A1C220A9000000,0xE2C925AF53A6358,0x1EA6350A03C8B51D,0x17458B22C2843},
{0xB5856,0x0,0x0,0x290033AFBE000000,0x42697FD96F210E5E,0x840E7DDC40F216E0,0x11A16DA8F12B4},
{0xC40AF,0x0,0x0,0x9D5EA53ED3000000,0x76A66D57E907B963,0xE976C6AE7E1B78A3,0xBFD502F1FD25},
{0xD2908,0x0,0x0,0x11BD16CDE8000000,0xAAE35AD662EE6469,0x4EDF0F80BB44DA66,0x65932B54E797},
{0xD9D34,0x0,0x0,0xCACD0AD2E4000000,0x82E4FFD1B88E113D,0x381163D51ABC1B73,0x15281D3121672},
{0xE1161,0x0,0x0,0x861B885CFD000000,0xDF204854DCD50F6E,0xB4475852F86E3C29,0xB5153B7D208}};

const uint64_t table_v_qr[TABLE_V_LEN][NWORDS64_FIELD] = 
{{0xB91B91B91B91BAE5,0x1B91B91B91B91B91,0x91B91B91B91B91B9,0xE6BB9DE0C91B91B,0x18879A47F2F0700D,0x32C54E927DB88A64,0x40658CB2BA6D},
{0x37237237237228D7,0x2372372372372372,0x7237237237237237,0x8A63A78A7E723723,0xB7898A3899D52E3B,0xD6D1EB429400CD35,0x30F2C1819FDA},
{0x9DF0A569883D6A65,0x59DF0A569883D6A5,0xA59DF0A569883D6A,0x93172790C79883D6,0x298B605E3417DFB7,0x81016F426986717,0x1AE3C15954874},
{0x26FFB1636401381F,0x726FFB16364013A7,0xA726FFB16364013A,0xEDF3527199364013,0xBDFF43E957D8E920,0x74E1DFE3B43CAA6,0x1E902BD6A68C3},
{0xB29534AE62D85A79,0x73A1CBC35EA5FE1F,0x1FB29534AE62D85A,0x19C5E5E8785EA5FE,0x6B6158A5E8F068E6,0xCECA7AEFD88C24A,0xCF8664C37D4D},
{0xC6092EF433268DDD,0x7A7EB10D61508715,0x15C6092EF433268F,0x83BED59416615087,0xDA8AD203B0BCEA94,0xDC0EF2686153C430,0x1217CC87DEA8B},
{0xA1597A9A1597A9A3,0xA9A1597A9A1597A9,0x97A9A1597A9A1597,0x47173487D07A9A15,0x3062918D06B8B8BB,0x5EF6E4246C346412,0x5CDEB10B32FD},
{0x5342B2F5342B2E6A,0x2F5342B2F5342B2F,0x2B2F5342B2F5342B,0x4305EAA744B2F534,0x71C32BD0E142A474,0xE23E2A390CBEBE56,0x20B54D66A6931},
{0x31D09D29093CBD5A,0xA279F28F044339E9,0x1C3591FDDFA2ABE3,0x7A95272488FA8224,0x7A5F257CDC0C28BB,0x2F3DFE9FC01A2996,0x1366E89ECB66C},
{0x79FC8C0827902CB2,0x2ED4817F4D755A6E,0x26199D587B0F39F3,0xA3A0E2151F79A526,0x1386C93E197B9F03,0x6B7E90520EBF0D2E,0x1E10076731598},
{0x5DD68140F43AA56B,0x4AA53DAEB55472EF,0xA60F1009929BD460,0x3D4DD25C8193C341,0x35733EAA8A1EA07B,0x99783B0926D5C757,0x1D6A91F47E8D5},
{0x1AC9583D8A91979F,0x71D48F5496803FF6,0xC01CD35C6E4198DD,0x941E3BB5DBE83352,0x421F7BB430992C92,0xDA40000C2EA98E76,0x23324E9FB029F},
{0x952DEB9E62D9C2BE,0x7B98EE26B6AE484E,0xA57575A4B923F963,0x7F6E921069A934B5,0xD34628FB53F37E0,0x84CD034670F66804,0x233E185011583},
{0x2E520464D0564CC2,0xF0B64C978A80BE93,0x6CCC6215FCC6ED62,0x3727B865F5F6FDE0,0xA5963EE8317E2563,0x3AEEFD45F2BC2A1D,0x8B27F67FAD8C},
{0x6B7A7DCC7F3B44FC,0xBA2952F78F79D85C,0x58AA10FC845A4938,0x38607CDC19E40DE8,0xF782E6EB3A1CBDE0,0x98DAE7F75A012666,0x1E9CFDCCA840A},
{0xF1F1925DF1F95FED,0xC765FD5CD394560F,0x918500D0FB76A868,0x5C3B831FAFBCCBDF,0xB1E4D8E96D477318,0xC4EB9AA3919CE98F,0xCA5C17F444A8},
{0xF505266D38669368,0xAD2B0AFD3D136E5D,0x3CA7F0C0A4792B2B,0xF6CC2F16221333E8,0x74FDE820A0C50733,0xD749F43FD6060D30,0xFF30BAB231A0},
{0xF7799FF6EEB44369,0x9C9B01C9BF32BB02,0xEE5B00B022443ED1,0x16418A8BDAD79B04,0x264771819C98C2EB,0x7CB8EF625BB1BCE,0xDC5C646444C8},
{0xA52C6022C70D3B7D,0xF9B79B1003C84AFA,0x765C85B2883915A,0x3C54570B229D2FC2,0xA59E1613FB354422,0x88D1CAF4B3C2622E,0x9178DE51F8AB},
{0xD5539351F6A61891,0xA23B6DF42E15B0BB,0xE1EDE32164D9C3B3,0xA7DECA9161CAC1A8,0x875FC6BEE0404B9D,0x907741C12EF25428,0x18B79DBCD1C48},
{0x18A1E0F1292838F7,0xBAE67B6B45CF66EC,0x2982C916B06A06F1,0x9A53622D7C86DA5A,0x9314B3E18DD3A3B7,0x36BB18009CF1A383,0x1BFC9236F3E65},
{0x22470120336BBA45,0x11EAC8697B6F6693,0xFB672FD432B4132A,0x27792DC88E63B55C,0x30D776047CAC287C,0xDE79BC44A0443180,0x1F651F5F1A9A7},
{0x3E09917AC9688DC1,0xEC94402B3F3397BE,0x56025A9144380945,0xA1AE22A2253250,0x891F8847B8B78A7D,0xDA0443EFF16C706F,0x22368A03A9304},
{0xD4F15775A9821722,0x64DF3D6397D527E8,0xF568724D03D64557,0x5B53D0D1B29D967C,0x7E629228475B7DAB,0xE2F8D30C52E943C7,0x493BB0D3CEFE},
{0x57EF7EC9AC7CE353,0xB563E360103A5DD6,0xC42C5BD044455DBC,0x41EA4366B0C69229,0xE97382F193D45A27,0x8B63E6E956D7C174,0x190B9B4F7F000},
{0x972A66E90C6F6DE7,0x8772CAA4BC06BFF,0xF5BB1C1771A8F362,0x7609CCF88A4C136B,0xB995935C846B7647,0xF7C843EA8188B335,0x1542E019C30FC},
{0x4246FA13382275B5,0x4734FBDCE5228591,0x4024F01C044310A0,0x764F2B4888774196,0x7F3EFA9545B4D464,0xCD48FEF1A7C24D0E,0x371FB2198997},
{0xD444A482CEF0AC6,0xDB7957048C8DE64A,0x1DC15465E53A2A4B,0x17829A8B38E47B7,0x1394CE02BBC42752,0x30B81EE68A7D7B3B,0x1B89AA7B09560},
{0x91E8F43CA01721C0,0x39ABE9BE4138C9F2,0x342B58F5DC8FD7C4,0x8BEBB3AD3C59CBD4,0x14A504F2E2D58BF,0x63B43021EDEEB875,0xCC6FC5A0F66},
{0xA9571A2B15CA47C5,0x136159ABA10347FA,0xC91B46A6CF975D38,0xC91B144F43D03B6F,0x764A2DFFD44DAC62,0x9DC2BEE6D762D341,0xA472F28BA413},
{0xE22367F18819D3B6,0x581661D0BA67F352,0xBB446CC1C435EC24,0x234DE0F57175551C,0x22DCB9526AB8B42A,0x9B132D68572887FF,0x106208C8243C0},
{0x61524ABE5BC5AA7B,0xE09DD59654379389,0x1E2CCF9B77486349,0x401627FB5217B2B5,0xA0ABBE33F68582CB,0x5FDCEBB71DBF474A,0x1A6CA9A638D47},
{0x743A7ADEC4CBEB9,0x47D5F3C3612D8380,0x2AB1A136944C1875,0x1A465F3FB9F85ABF,0xEE93D67ED7FD59F4,0xAECA171A0F106B4E,0x58273E8B0400},
{0xCC45DEE3E386734,0x9A5A0BDFB68C83AE,0x5C1D5047058C14ED,0x9FB2AA0ED2404366,0x166F7D427E758FC3,0x95C498C32ED2863F,0x18C4775225D1C}};

const uint64_t table_v_qnr[TABLE_V_LEN][NWORDS64_FIELD] = 
{{0x6666666666667DA2,0x6666666666666666,0x6666666666666666,0xBDB6F9CBC0666666,0xC31C792F24DD0205,0x27089D99E099E6BE,0x11108FDFC0447},
{0x33333333333304BB,0x3333333333333333,0x3333333333333333,0x825382E362333333,0xF58D6A19E79EAA98,0x1EEB24A2C09152D8,0x120D2B1F6AB6},
{0x8B6BE9F1D2505894,0x38B6BE9F1D250583,0x838B6BE9F1D25058,0x9C0909AB631D2505,0xEA75FE266DA83956,0xB98D22E1CCB2BBBE,0x7DB423F53C56},
{0x32698CFF3659C593,0x32698CFF3659CC0,0xC032698CFF3659CC,0xFC24A25976F3659C,0xEE93EFA540E7611C,0x15016B5024486E43,0x22DF13C37023D},
{0x7722377223772254,0x3772237722377223,0x2377223772237722,0x36D7BAAAFB223772,0x7AA4C5D72FC85655,0x3369C65060D08696,0x104009788DC36},
{0x1BB911BB911BB571,0x11BB911BB911BB91,0x911BB911BB911BB9,0x35E9BD3E8B911BB,0xEC06B024EB2770F5,0xF590D385807C1241,0x8DC059443A3B},
{0xE6111C7042F3A8E9,0xA8E6111C7042F3A8,0xF3A8E6111C7042F3,0xE2F2B515761C7042,0x530947C450F471E0,0xFE9688F7B8604C8D,0x5C0ECF10587A},
{0xED731D065EB956BE,0x57ED731D065EB957,0xB957ED731D065EB9,0xFB2E1576F91D065E,0xF2D84E5C17F75228,0x5A258E938774B74D,0x4472D355531},
{0x3337C993650C0D08,0xD089EDE9FF52557D,0x7620D4C829513EF7,0xDA1743C426DBF666,0xA107EC0033174F2C,0xAD15D89173C23C09,0xF99F9FB462F5},
{0xFC6A84D90E95D167,0x143E213087AD362F,0xB659C39FB886CE65,0xD6153F7A8F277FF3,0x39C5C5A13D9EDBF8,0xAAAC1FF0ADC9568B,0x11A1FD239C2DA},
{0x87736309DDA6A943,0x91F6A7837911C71,0x743D3FD9AC58005,0xE763272C18E09F9A,0x26B8C4C17D8D30CA,0xA460F26C8AFDB3DA,0x14E6984FF9B2E},
{0xF4EC60AC7873FE1A,0x604D5A5B78D31CAD,0x21E19A43B14CFB3D,0x1179D702BA92065,0x4EA04B3126A1C70,0xEA50BE0D9B0E2C6D,0x131C643902465},
{0x8614211F80B1616D,0xF7B81FD3A7A49E4E,0xF80B1616D86C5E7A,0x7BF548F41614211,0xC37DA826EB2AE8DF,0xC0411FD8C5ED8C6F,0x1F341510F9157},
{0x295ABC8F38726461,0x50DC31E366CDE7A9,0xF387264C8615B5A9,0xC75E33E8F95ABC8,0x5D38EFF0970FFA71,0x461FA2CC981D3869,0x418CBED84E70},
{0x87700C2E778D8075,0x43A87500FA58EEAF,0x9CE75275C96D2A5C,0x5FA19F2E462F399A,0xDF0F017B7DD8C4F2,0x280013794FEDAE61,0x94ED77FF8063},
{0x9C6D58D8EF534C70,0x660CD680A7D28338,0xBDC9BBA390D722B8,0xA71C6310BFAFCB3F,0xB99B4BBB58195F7C,0x6691D6254221D44,0x12108224A3BF7},
{0x2195F504EC8CE821,0x9D41075B38625AA5,0x9014203CA406BB05,0x6D62CB6D8B2DF2C3,0x1C86F3E3A5CCF863,0x203E5B5903F00EDA,0x164F4C3C7C43C},
{0xF6674D58304FF4E1,0x93B111A6E31CA9BA,0xBC9F5567AC2B4021,0x38B75040103B443C,0xA69C3BA57D6DCDCB,0xA75A1F558C7AE4E1,0x906E5FF4617C},
{0x94129AFB594129AF,0x129AFB594129AFB5,0x9AFB594129AFB594,0x2900F1CD82B59412,0x20D6A4CA5E4BAF82,0x6793F3042CF7FB07,0x1D02041A5E76A},
{0xDACA094D7DACA05A,0xCA094D7DACA094D7,0x94D7DACA094D7DA,0x4B05B38CC6D7DACA,0x5BB2D964C0D0DE02,0x801FF33FFAFD404E,0x1AAD724D92DFC},
{0xE166CDB3BE58BCD0,0xE77C60E75642CDFD,0xC39472A9783BE7F,0x96A7FC14FBC96ABA,0x5CC689CB13C56A4D,0xC328410D8CB268EB,0x1A8A8A5004ABD},
{0xB346305840760AA7,0xB1B19541D3651803,0xC76AE6F315D7AC5C,0xF0B9349859647081,0xA9244A61C30F5B9B,0x6A76EE9B87BC5C4F,0xAF0668ACA050},
{0xADA031682113D6D1,0x17CF7EC83494C4A,0x33EF143065596ED1,0x276B5923C8389FE,0x822C190F53979AC,0x33C32FAC0249AB9F,0x11975648F1F39},
{0x6FF3CFA74C02322E,0xD98760896B12B35E,0x5DEE8FFD24640437,0x3069F716185FB36D,0xA97ABED04F5F5959,0xDBDAA58A321B6B1F,0x491708DEB2B8},
{0xA6D19EFB46E8A47A,0xB4DDF0D82FF93FF2,0x7EF791FD729C9B0F,0x1CBEBA7707E84CC6,0xE949FCCBE6E9F9A6,0x85AFF3FCD75590B,0xF96FFCFE49F3},
{0x7475B90FC40CE223,0xDC4E0065C0F5C12D,0xB2928765F52DD4E,0xCB9E763FBEB91EC8,0x44396405161C4326,0xA0D8B0D51BD60EA8,0xA27A2002EA20},
{0x49EC0E19DF71A633,0x2A457AC271C844D8,0xDBF40F545519A32C,0x318D60D527180CFE,0x19A398A08B929CDA,0xCF17763B94DCBD5B,0x1DD4F8A136584},
{0xFD97B0A56ABB6CB8,0x404AD893F967E82A,0x4C00B1B353FCAE77,0xF75B9E618F05FC7F,0x9C6E7902F4787FCE,0xE4E698A7B7077008,0xCF2A0C917171},
{0xC0BAAADC355770BB,0x9D7581EB3277D7EA,0xCDBA0DAFF2A59F6E,0x3E461B402A9B65B2,0x5F0D156E1A7A3ED,0x69A1891EBDCC6A7C,0x1281AFCFED68C},
{0xB5896529877D4FE7,0x71F064AE68975B99,0x4F9211F7BBCE7B39,0x19AC9E4507FDCF34,0xE4F185BB956F52E1,0xE327A4E691DAE76D,0x46219384EE75},
{0x1B49FE6D324A903E,0x4E33D89D1655631F,0x1E1C101410671389,0xE332832DFB6C742D,0x38CDD49E02180B90,0x1279178D8097B588,0x119E5683A4805},
{0x1FBB103663B9C88C,0x2374EF7AF79F0D48,0x4AAEF2ECAB3EA292,0xA0FA2CB6396F22C4,0xBE1E1F8EC2B4A5CA,0x43D353300E514A3B,0x1288094A6D8E5},
{0x998BF5138DDDD5D2,0xD1F015AF4EA7BF66,0xE3DAF7886068D9AD,0x99E9067025D583AF,0xCC498E253E868EF6,0x618AE02455C9B20A,0x2322145632832},
{0x333A0330E480A5E0,0xD37D31D37894FFB3,0x4E09921C2CCDB4FD,0xAC7069BEE6F95173,0xA05EDFF1E57DBBEF,0x5C3678A72B135234,0x1BB5DA10EE9DF}};

const uint64_t v_3_torsion[20][2 * NWORDS64_FIELD] = 
{{0x999999999999BC73,0x9999999999999999,0x9999999999999999,0x9C9276B1A0999999,0x24AAB5C6B74B8308,0xBA8CEC66D0E6DA1E,0x1998D7CFA066A,0xCCCCCCCCCCCCC12E,0xCCCCCCCCCCCCCCCC,0xCCCCCCCCCCCCCCCC,0x1EE5F99502CCCCCC,0x1A381FE09EEA2DA1,0xD978110991782CF7,0x1AB9AA8197120},
{0x41FAB8BE05474CC2,0x54741FAB8BE0547,0xB8BE054741FAB8BE,0xC9AE7F39FF4741FA,0xB1F4EE9D1CDDE556,0x1650D500E95EBA31,0x5023E95DDE16,0x54741FAB8BE04FA9,0x8BE054741FAB8BE0,0x1FAB8BE054741FAB,0x16655C7EB0E05474,0xD849DDDEC437B020,0x2E677A183619E2D7,0x19324B6AFA19D},
{0x8536718536718A34,0x3671853671853671,0x7185367185367185,0x88EBEF311A718536,0xA7D32684ABC9D6E9,0x86FFA027C5EA41ED,0x1EF2D26807E04,0x8B01288B012888A3,0x1288B01288B0128,0x288B01288B01288B,0xF2CAF7B478288B01,0xB30406DB0864DD0C,0x7DE19AB6317E9551,0x1499080A4B0D6},
{0x497819E2B03FF6AB,0x43C254E3A3E0C8FE,0xA1F98753F0030B9C,0x9D08AFDF66CDC06D,0xDC9B4D2060AC66D4,0xAA1B0432076D8C12,0x8C728C65FAC7,0x7787B5638B83E57E,0x4BC0CF1581FF9E8C,0x1E12A71D1F0647F2,0xF874D414C6185CE2,0xB1C3CEFB8EF40E5E,0xD974CD64813E2AD0,0xEF9B64A50B19},
{0x4FAC336C14F326D0,0xF324FAC336C14F32,0xC14F324FAC336C14,0x7A441A68B4FAC336,0xF55ABB9E02E51BCD,0x7DF241E5CC125D82,0x20CD99B418553,0x79927D619B60A6B2,0x60A79927D619B60A,0x19B60A79927D619B,0xC1836ABCD19927D6,0x56ED5F9FA770E50,0x938A248852AF05FF,0x180454EC85AD4},
{0x8FC8F97836B765C9,0x79F3DDD74597E657,0xF40F4D525855A12C,0xB504F4CAE1D399D4,0x5B3EBD1F2ED83FF2,0xE84645130B2D0DBA,0xDA2062334ED3,0xC71EA489199E8B2C,0x4CA048F486023DBF,0x7D1F0775331F6A61,0x238045677D90A29A,0xA9A61EA8DBC1A3F4,0x7D71C1C648CD9742,0x1075C2F69D7C4},
{0xC3A939DD014D003A,0xA92F1F17B5248C25,0x7399C8DD37028F40,0x78F15EE9E1AEC3C9,0x9C6EC31DE809B64F,0xADC32BB57B097A2B,0x182925F9D25CA,0x37B92BC3BEB5AFB9,0xC389002039BCC790,0x769F328FEC87C89C,0x45F48B81136118A1,0x771813745D6BBD3E,0x1DB0C12AB3EBFB1D,0x14733DD55FF54},
{0x8D3D7757053F25DE,0x1BB0A5534AD2D44F,0x6A5D806D0822AA3D,0x81D30F00C5FE0658,0xDD7EDBA5455F0CBF,0x321F8C3287B5561C,0x10C3B9266397A,0x58AFE48B66934683,0x2BD00D3BE2D0F3F0,0x366476DBC465DF23,0xCF46D882997C04CA,0x100047EAE7B1B0CA,0xE98B984D9BA8857B,0x19D8D87E9F5E7},
{0x16255E514230CE8B,0x4CEE6DB1C53E2C2B,0xE69FC4293B640916,0xBEDB813769959101,0x75374AFDF84FA267,0xA08A7C370DEA787F,0x1C42814BBDFF3,0xCA970CCFC4688815,0xB0EF9A5F6A27C77F,0x57FF227293F58D8B,0x2782596E2D3675FA,0xF3B4D13FBB928B8E,0x5F9B384AE0301D97,0x15DD020A61A84},
{0xB9758CA6605A88DC,0x9B0E00E773B0BE3F,0x8D1CB3F2BBA35FE1,0x5BD852F804F988BA,0x5AB812185EC1F54A,0x81EE7303B3B11EF8,0x167A5C4B8EC00,0x537E31BF0E7887B0,0xF28A3A23236AEF0A,0x49B722040DA1FB99,0x89C14F4BF6ADE22F,0x8303646A89C2A6D6,0x4446EB8898043332,0x1C1BE6A2BAA3F},
{0x60904BED92E6F0A1,0xB75830F83D32BDCE,0x5303D79E3A35E71C,0x57F1D1FBF1F6424,0x9D29AFFCD4B9F82,0xDE6CF6ECF1EDB33,0x1B330E4A5A838,0x9DBF7B0AE6C58C5,0x45E3E366E4AFF3A9,0x29E3BFD3FA3102E9,0x4F66726AC8918DDF,0xAC46D1291B0F682A,0x197E0510E123E22F,0x141E2A18F39E4},
{0x611E67F5AFE0C95,0x73F50769BEADF858,0x5FF1EF605FF0AD88,0xC80A90794F63FDDE,0xA435619598EC3001,0x34AF362A0C54F5EA,0x17A9719011B55,0xE183ECD15F816A3A,0xF6F117C4395997BE,0x67AAE2316B369528,0x618723D3BFE02D5,0x947CE54AACCA22AF,0x4963179F500215D8,0xA69EB2B134C5},
{0x4512C7CB549E0613,0x58B3138052AC291F,0xBDB21A1D0DC8E1F3,0x8F33DE426C49F38,0x78ED469B002924BC,0x94FBCA884C534ECE,0x16411608F666E,0x76DCF643092ACD65,0x87827A6E7863F18A,0x5FBB99DF645C6BDD,0x26E3468335753344,0xC6BB91D8121B199B,0x3BC17926E87E395D,0x7D5A5FF0E692},
{0x2CD377F7AF7A3996,0xFE915A6848EB6335,0xE2796F2ACB922496,0xD5FEAE7013EC2D63,0x1B264179788453F2,0x3A50528DF123CA80,0x3C2B61484D9B,0xCD1CFE4E128028C,0x7E6F33F849117206,0x422F91D71D67A4CB,0xA9C6E809C409E2D9,0xD3D9B2D66BAC46C7,0x68CEEB5277B8DDD3,0x381E05A78010},
{0x4F4EBAA4A85CBB5A,0xF6A3476ECCE55C,0x6DF6C3C9230F72FA,0xDCB283C6564F0BA,0xA82B8AC12EDC42E9,0x5FBD7E3CB399C776,0xFFF6BD7C7E3E,0xDC68551ECFC41D23,0xC237308737FA469E,0xE6A81586A6FCD565,0x3DED4572B5A03B45,0x6946056BDF996305,0xE2F1389B1D6CF35C,0x10CD02839AEB3},
{0xA5959A668F12A176,0xB310CEBDA555CD7D,0x78B28C38E9A01DF,0x6197E0DCCE65322A,0xA8D2A1E798A9702A,0x753603AD399F0CAA,0x105A74C730C51,0xC8D0CA6784B4551A,0x45D4AE4A0850F0C8,0x391DDCAFE0C29DC1,0x8270BF6B173AC986,0x73770F0EAA80DDEF,0xBB7C9EE74BD61C23,0x7A592BC0228D},
{0xA302403B141A0385,0x9188DF46965BF724,0x666234B05EB3A268,0x37AB02DCD5D0FB01,0xED9FA4ABF47BF81A,0x31B87D353910E9E3,0x1E931A394CEF8,0x9E46F21160F6FEAF,0x6359F8041E2E0FEA,0x4AE82F18B6A502F1,0xCC2D17C159C53B16,0xDC5076C57AEF74C5,0xA4D66592DBF0E267,0x12E32E3B2202},
{0x9237E2347B57BBD8,0xE16C142FE156484A,0x87E8A48B3B14220,0x34B6225A95A547ED,0xD26CBF294A3E2E72,0x33C06874E35A7F57,0x17DFC7A25D7B7,0x91ECC77A8220B97E,0xBD3818A24751F785,0x2E3FD75D7AE703AB,0xD7E8692B70CA482C,0x53C8B5DDEBA21D6D,0x5FC1E218D7085E99,0x129CD0E49FE6F},
{0x3945471CC48EF6BB,0x7C3FB717D0165DC8,0x92F83793BFEBC75A,0xD5BF95D93BFE5316,0x209E198DB1F16A4,0xE2086C62914F403A,0x19D6352AA2CC,0xBFFF0BF092E15611,0xFA429EEA322007B3,0x99EFB4184EBBD84C,0xF6C5230835D383A8,0xF1CD8283C93674AF,0x41C64F9EA1AC8458,0x2CD68D0A2571},
{0x570C567E928211B1,0x70459EE4560443C6,0x1FC204B885DBB2C0,0x7C99D3BCAF19BE0E,0x5BFFD67B5AA530E7,0xF8701F7AE35B9C57,0xF056DF1DFCBE,0x4183962A42D28AD1,0xFF8B6A5752A5FAEB,0xA4B714F92FE41B11,0x8CC5D5EEFFAFB58B,0xA53A15C837E4CFC9,0x1A79CF6122569F16,0x1FE4DB81AE342}};

// Setting up macro defines and including GF(p), GF(p^2), curve, isogeny and kex functions
#define fpcopy                        fpcopy434
#define fpzero                        fpzero434
#define fpadd                         fpadd434
#define fpsub                         fpsub434
#define fpneg                         fpneg434
#define fpdiv2                        fpdiv2_434
#define fpcorrection                  fpcorrection434
#define fpmul_mont                    fpmul434_mont
#define fpsqr_mont                    fpsqr434_mont
#define fpinv_mont                    fpinv434_mont
#define fpinv_chain_mont              fpinv434_chain_mont
#define fpinv_mont_bingcd             fpinv434_mont_bingcd
#define fp2copy                       fp2copy434
#define fp2zero                       fp2zero434
#define fp2add                        fp2add434
#define fp2sub                        fp2sub434
#define fp2neg                        fp2neg434
#define fp2div2                       fp2div2_434
#define fp2correction                 fp2correction434
#define fp2mul_mont                   fp2mul434_mont
#define fp2sqr_mont                   fp2sqr434_mont
#define fp2inv_mont                   fp2inv434_mont
#define fp2inv_mont_bingcd            fp2inv434_mont_bingcd
#define fpequal_non_constant_time     fpequal434_non_constant_time
#define mp_add_asm                    mp_add434_asm
#define mp_subaddx2_asm               mp_subadd434x2_asm
#define mp_dblsubx2_asm               mp_dblsub434x2_asm
#define random_mod_order_A            random_mod_order_A_SIDHp434
#define random_mod_order_B            random_mod_order_B_SIDHp434
#define EphemeralKeyGeneration_A      EphemeralKeyGeneration_A_SIDHp434_Compressed
#define EphemeralKeyGeneration_B      EphemeralKeyGeneration_B_SIDHp434_Compressed
#define EphemeralSecretAgreement_A    EphemeralSecretAgreement_A_SIDHp434_Compressed
#define EphemeralSecretAgreement_B    EphemeralSecretAgreement_B_SIDHp434_Compressed 
#define crypto_kem_keypair            crypto_kem_keypair_SIKEp434_compressed
#define crypto_kem_enc                crypto_kem_enc_SIKEp434_compressed
#define crypto_kem_dec                crypto_kem_dec_SIKEp434_compressed
#define cryptorun_benchs              cryptorun_benchs_SIKEp434_compressed


#include "../fpx.c"
#include "../ec_isogeny.c"
#include "../compression/torsion_basis.c"
#include "P434_compressed_pair_tables.c"
#include "../compression/pairing.c"
#include "P434_compressed_dlog_tables.c"
#include "../compression/dlog.c"
#include "../compression/sidh_compressed.c"
#include "../compression/sike_compressed.c"
