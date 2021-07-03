//
// Created by levigibson on 5/29/21.
//

#include <stdio.h>
#include "bitboard.h"

void print_bitboard(U64 bitboard){

    int rank = 8;
    for (int square = 0; square < 64; square++){
        if (square % 8 == 0) {
            printf("\n");
            printf("%d  ", rank);
            rank--;
        }
        if ((bitboard >> square)&1ULL){
            printf("1  ");
        } else{
            printf(".  ");
        }
    }

    printf("\n   A  B  C  D  E  F  G  H");

    printf("\n\nBitboard: 0x%llxULL\n\n", bitboard);

}

const U64 knight_mask[64] = {
        132096ULL,
        329728ULL,
        659712ULL,
        1319424ULL,
        2638848ULL,
        5277696ULL,
        10489856ULL,
        4202496ULL,
        33816580ULL,
        84410376ULL,
        168886289ULL,
        337772578ULL,
        675545156ULL,
        1351090312ULL,
        2685403152ULL,
        1075839008ULL,
        8657044482ULL,
        21609056261ULL,
        43234889994ULL,
        86469779988ULL,
        172939559976ULL,
        345879119952ULL,
        687463207072ULL,
        275414786112ULL,
        2216203387392ULL,
        5531918402816ULL,
        11068131838464ULL,
        22136263676928ULL,
        44272527353856ULL,
        88545054707712ULL,
        175990581010432ULL,
        70506185244672ULL,
        567348067172352ULL,
        1416171111120896ULL,
        2833441750646784ULL,
        5666883501293568ULL,
        11333767002587136ULL,
        22667534005174272ULL,
        45053588738670592ULL,
        18049583422636032ULL,
        145241105196122112ULL,
        362539804446949376ULL,
        725361088165576704ULL,
        1450722176331153408ULL,
        2901444352662306816ULL,
        5802888705324613632ULL,
        11533718717099671552ULL,
        4620693356194824192ULL,
        288234782788157440ULL,
        576469569871282176ULL,
        1224997833292120064ULL,
        2449995666584240128ULL,
        4899991333168480256ULL,
        9799982666336960512ULL,
        1152939783987658752ULL,
        2305878468463689728ULL,
        1128098930098176ULL,
        2257297371824128ULL,
        4796069720358912ULL,
        9592139440717824ULL,
        19184278881435648ULL,
        38368557762871296ULL,
        4679521487814656ULL,
        9077567998918656ULL
};

const U64 king_mask[64] = {
        770ULL,
        1797ULL,
        3594ULL,
        7188ULL,
        14376ULL,
        28752ULL,
        57504ULL,
        49216ULL,
        197123ULL,
        460039ULL,
        920078ULL,
        1840156ULL,
        3680312ULL,
        7360624ULL,
        14721248ULL,
        12599488ULL,
        50463488ULL,
        117769984ULL,
        235539968ULL,
        471079936ULL,
        942159872ULL,
        1884319744ULL,
        3768639488ULL,
        3225468928ULL,
        12918652928ULL,
        30149115904ULL,
        60298231808ULL,
        120596463616ULL,
        241192927232ULL,
        482385854464ULL,
        964771708928ULL,
        825720045568ULL,
        3307175149568ULL,
        7718173671424ULL,
        15436347342848ULL,
        30872694685696ULL,
        61745389371392ULL,
        123490778742784ULL,
        246981557485568ULL,
        211384331665408ULL,
        846636838289408ULL,
        1975852459884544ULL,
        3951704919769088ULL,
        7903409839538176ULL,
        15806819679076352ULL,
        31613639358152704ULL,
        63227278716305408ULL,
        54114388906344448ULL,
        216739030602088448ULL,
        505818229730443264ULL,
        1011636459460886528ULL,
        2023272918921773056ULL,
        4046545837843546112ULL,
        8093091675687092224ULL,
        16186183351374184448ULL,
        13853283560024178688ULL,
        144959613005987840ULL,
        362258295026614272ULL,
        724516590053228544ULL,
        1449033180106457088ULL,
        2898066360212914176ULL,
        5796132720425828352ULL,
        11592265440851656704ULL,
        4665729213955833856ULL
};

const U64 pawn_mask[2][64] = {
        {0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                2ULL,
                5ULL,
                10ULL,
                20ULL,
                40ULL,
                80ULL,
                160ULL,
                64ULL,
                512ULL,
                1280ULL,
                2560ULL,
                5120ULL,
                10240ULL,
                20480ULL,
                40960ULL,
                16384ULL,
                131072ULL,
                327680ULL,
                655360ULL,
                1310720ULL,
                2621440ULL,
                5242880ULL,
                10485760ULL,
                4194304ULL,
                33554432ULL,
                83886080ULL,
                167772160ULL,
                335544320ULL,
                671088640ULL,
                1342177280ULL,
                2684354560ULL,
                1073741824ULL,
                8589934592ULL,
                21474836480ULL,
                42949672960ULL,
                85899345920ULL,
                171798691840ULL,
                343597383680ULL,
                687194767360ULL,
                274877906944ULL,
                2199023255552ULL,
                5497558138880ULL,
                10995116277760ULL,
                21990232555520ULL,
                43980465111040ULL,
                87960930222080ULL,
                175921860444160ULL,
                70368744177664ULL,
                562949953421312ULL,
                1407374883553280ULL,
                2814749767106560ULL,
                5629499534213120ULL,
                11258999068426240ULL,
                22517998136852480ULL,
                45035996273704960ULL,
                18014398509481984ULL},
        {512ULL,
                1280ULL,
                2560ULL,
                5120ULL,
                10240ULL,
                20480ULL,
                40960ULL,
                16384ULL,
                131072ULL,
                327680ULL,
                655360ULL,
                1310720ULL,
                2621440ULL,
                5242880ULL,
                10485760ULL,
                4194304ULL,
                33554432ULL,
                83886080ULL,
                167772160ULL,
                335544320ULL,
                671088640ULL,
                1342177280ULL,
                2684354560ULL,
                1073741824ULL,
                8589934592ULL,
                21474836480ULL,
                42949672960ULL,
                85899345920ULL,
                171798691840ULL,
                343597383680ULL,
                687194767360ULL,
                274877906944ULL,
                2199023255552ULL,
                5497558138880ULL,
                10995116277760ULL,
                21990232555520ULL,
                43980465111040ULL,
                87960930222080ULL,
                175921860444160ULL,
                70368744177664ULL,
                562949953421312ULL,
                1407374883553280ULL,
                2814749767106560ULL,
                5629499534213120ULL,
                11258999068426240ULL,
                22517998136852480ULL,
                45035996273704960ULL,
                18014398509481984ULL,
                144115188075855872ULL,
                360287970189639680ULL,
                720575940379279360ULL,
                1441151880758558720ULL,
                2882303761517117440ULL,
                5764607523034234880ULL,
                11529215046068469760ULL,
                4611686018427387904ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL}
};

//rook magic numbers
const U64 rook_magic_numbers[64] = {
        0x8a80104000800020ULL,
        0x140002000100040ULL,
        0x2801880a0017001ULL,
        0x100081001000420ULL,
        0x200020010080420ULL,
        0x3001c0002010008ULL,
        0x8480008002000100ULL,
        0x2080088004402900ULL,
        0x800098204000ULL,
        0x2024401000200040ULL,
        0x100802000801000ULL,
        0x120800800801000ULL,
        0x208808088000400ULL,
        0x2802200800400ULL,
        0x2200800100020080ULL,
        0x801000060821100ULL,
        0x80044006422000ULL,
        0x100808020004000ULL,
        0x12108a0010204200ULL,
        0x140848010000802ULL,
        0x481828014002800ULL,
        0x8094004002004100ULL,
        0x4010040010010802ULL,
        0x20008806104ULL,
        0x100400080208000ULL,
        0x2040002120081000ULL,
        0x21200680100081ULL,
        0x20100080080080ULL,
        0x2000a00200410ULL,
        0x20080800400ULL,
        0x80088400100102ULL,
        0x80004600042881ULL,
        0x4040008040800020ULL,
        0x440003000200801ULL,
        0x4200011004500ULL,
        0x188020010100100ULL,
        0x14800401802800ULL,
        0x2080040080800200ULL,
        0x124080204001001ULL,
        0x200046502000484ULL,
        0x480400080088020ULL,
        0x1000422010034000ULL,
        0x30200100110040ULL,
        0x100021010009ULL,
        0x2002080100110004ULL,
        0x202008004008002ULL,
        0x20020004010100ULL,
        0x2048440040820001ULL,
        0x101002200408200ULL,
        0x40802000401080ULL,
        0x4008142004410100ULL,
        0x2060820c0120200ULL,
        0x1001004080100ULL,
        0x20c020080040080ULL,
        0x2935610830022400ULL,
        0x44440041009200ULL,
        0x280001040802101ULL,
        0x2100190040002085ULL,
        0x80c0084100102001ULL,
        0x4024081001000421ULL,
        0x20030a0244872ULL,
        0x12001008414402ULL,
        0x2006104900a0804ULL,
        0x1004081002402ULL
};

//bishop magic numbers
const U64 bishop_magic_numbers[64] = {
        0x40040844404084ULL,
        0x2004208a004208ULL,
        0x10190041080202ULL,
        0x108060845042010ULL,
        0x581104180800210ULL,
        0x2112080446200010ULL,
        0x1080820820060210ULL,
        0x3c0808410220200ULL,
        0x4050404440404ULL,
        0x21001420088ULL,
        0x24d0080801082102ULL,
        0x1020a0a020400ULL,
        0x40308200402ULL,
        0x4011002100800ULL,
        0x401484104104005ULL,
        0x801010402020200ULL,
        0x400210c3880100ULL,
        0x404022024108200ULL,
        0x810018200204102ULL,
        0x4002801a02003ULL,
        0x85040820080400ULL,
        0x810102c808880400ULL,
        0xe900410884800ULL,
        0x8002020480840102ULL,
        0x220200865090201ULL,
        0x2010100a02021202ULL,
        0x152048408022401ULL,
        0x20080002081110ULL,
        0x4001001021004000ULL,
        0x800040400a011002ULL,
        0xe4004081011002ULL,
        0x1c004001012080ULL,
        0x8004200962a00220ULL,
        0x8422100208500202ULL,
        0x2000402200300c08ULL,
        0x8646020080080080ULL,
        0x80020a0200100808ULL,
        0x2010004880111000ULL,
        0x623000a080011400ULL,
        0x42008c0340209202ULL,
        0x209188240001000ULL,
        0x400408a884001800ULL,
        0x110400a6080400ULL,
        0x1840060a44020800ULL,
        0x90080104000041ULL,
        0x201011000808101ULL,
        0x1a2208080504f080ULL,
        0x8012020600211212ULL,
        0x500861011240000ULL,
        0x180806108200800ULL,
        0x4000020e01040044ULL,
        0x300000261044000aULL,
        0x802241102020002ULL,
        0x20906061210001ULL,
        0x5a84841004010310ULL,
        0x4010801011c04ULL,
        0xa010109502200ULL,
        0x4a02012000ULL,
        0x500201010098b028ULL,
        0x8040002811040900ULL,
        0x28000010020204ULL,
        0x6000020202d0240ULL,
        0x8918844842082200ULL,
        0x4010011029020020ULL
};

const int rook_relevent_occupancy_count[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
};

const int bishop_relevent_occupancy_count[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;

const U64 not_hg_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;

U64 rook_attacks[64][4096];
U64 bishop_attacks[64][512];

/**
void init_knight_mask(){
    for (int square = 0; square < 64; square++){
        U64 bitboard = 1ULL << square;

        if (bitboard & not_a_file) knight_mask[square] |= bitboard >> 17;
        if (bitboard & not_ab_file) knight_mask[square] |= bitboard >> 10;
        if (bitboard & not_h_file) knight_mask[square] |= bitboard >> 15;
        if (bitboard & not_hg_file) knight_mask[square] |= bitboard >> 6;

        if (bitboard & not_h_file) knight_mask[square] |= bitboard << 17;
        if (bitboard & not_hg_file) knight_mask[square] |= bitboard << 10;
        if (bitboard & not_a_file) knight_mask[square] |= bitboard << 15;
        if (bitboard & not_ab_file) knight_mask[square] |= bitboard << 6;
    }
}**/
/**
void init_pawn_mask(){
    for (int square = 0; square < 64; square++){
        U64 bitboard = 1ULL << square;

        if (bitboard & not_a_file) pawn_mask[white][square] |= bitboard >> 9;
        if (bitboard & not_h_file) pawn_mask[white][square] |= bitboard >> 7;

        if (bitboard & not_h_file) pawn_mask[black][square] |= bitboard << 9;
        if (bitboard & not_a_file) pawn_mask[black][square] |= bitboard << 7;
    }
}**/
/**
void init_king_mask(){
    for (int square = 0; square < 64; square++){
        U64 bitboard = 1ULL << square;
        king_mask[square] |= bitboard << 8;
        king_mask[square] |= bitboard >> 8;

        if (bitboard & not_a_file){
            king_mask[square] |= bitboard >> 1;
            king_mask[square] |= bitboard >> 9;
            king_mask[square] |= bitboard << 7;
        }

        if (bitboard & not_h_file){
            king_mask[square] |= bitboard << 1;
            king_mask[square] |= bitboard << 9;
            king_mask[square] |= bitboard >> 7;
        }
    }
}**/

void init_rook_relevant_occupancies(){
    for (int square = 0; square < 64; square++){
        int rank = square / 8;
        for (int file = (square % 8)+1; file < 7; file++){
            rook_relevant_occupancies[square] |= 1ULL << ((rank*8)+file);
        }
        for (int file = (square % 8)-1; file > 0; file--){
            rook_relevant_occupancies[square] |= 1ULL << ((rank*8)+file);
        }
        int file = square % 8;
        for (rank = (square/8)+1; rank < 7; rank++){
            rook_relevant_occupancies[square] |= 1ULL << ((rank*8)+file);
        }
        for (rank = (square/8)-1; rank > 0; rank--){
            rook_relevant_occupancies[square] |= 1ULL << ((rank*8)+file);
        }
    }
}

void init_bishop_relevant_occupancies(){
    for (int square = 0; square < 64; square++){
        int sfile = square  % 8;
        int srank = square / 8;

        int rank, file;

        for (file = sfile+1, rank = srank+1; file < 7 && rank < 7; file++, rank++) bishop_relevant_occupancies[square] |= (1ULL << ((rank*8)+file));
        for (file = sfile-1, rank = srank+1; file > 0 && rank < 7; file--, rank++) bishop_relevant_occupancies[square] |= (1ULL << ((rank*8)+file));

        for (file = sfile+1, rank = srank-1; file < 7 && rank > 0; file++, rank--) bishop_relevant_occupancies[square] |= (1ULL << ((rank*8)+file));
        for (file = sfile-1, rank = srank-1; file > 0 && rank > 0; file--, rank--) bishop_relevant_occupancies[square] |= (1ULL << ((rank*8)+file));
    }
}

U64 rook_attacks_on_the_fly(int square, U64 occupancies){

    U64 moves = 0ULL;

    int rank = square / 8;
    for (int file = (square % 8)+1; file <= 7; file++){
        int target = ((rank*8)+file);
        moves |= 1ULL << target;
        if (get_bit(occupancies, target)) break;
    }
    for (int file = (square % 8)-1; file >= 0; file--){
        int target = ((rank*8)+file);
        moves |= 1ULL << target;
        if (get_bit(occupancies, target)) break;
    }
    int file = square % 8;
    for (rank = (square/8)+1; rank <= 7; rank++){
        int target = ((rank*8)+file);
        moves |= 1ULL << target;
        if (get_bit(occupancies, target)) break;
    }
    for (rank = (square/8)-1; rank >= 0; rank--){
        int target = ((rank*8)+file);
        moves |= 1ULL << target;
        if (get_bit(occupancies, target)) break;
    }

    return moves;
}

U64 bishop_attacks_on_the_fly(int square, U64 occupancies){
    int sfile = square  % 8;
    int srank = square / 8;

    int rank, file;

    U64 moves = 0ULL;

    for (file = sfile+1, rank = srank+1; file <= 7 && rank <= 7; file++, rank++) {
        int target = ((rank * 8) + file);
        moves |= (1ULL << target);
        if (get_bit(occupancies, target)) break;
    }
    for (file = sfile-1, rank = srank+1; file >= 0 && rank <= 7; file--, rank++) {
        int target = ((rank * 8) + file);
        moves |= (1ULL << target);
        if (get_bit(occupancies, target)) break;
    }
    for (file = sfile+1, rank = srank-1; file <= 7 && rank >= 0; file++, rank--) {
        int target = ((rank * 8) + file);
        moves |= (1ULL << target);
        if (get_bit(occupancies, target)) break;
    }
    for (file = sfile-1, rank = srank-1; file >= 0 && rank >= 0; file--, rank--) {
        int target = ((rank * 8) + file);
        moves |= (1ULL << target);
        if (get_bit(occupancies, target)) break;
    }

    return moves;
}

//this function is not important. It's for initialisation of magics.
U64 set_occupancy(int index, U64 mask){
    U64 occupancies = 0ULL;
    int bits_in_mask = count_bits(mask);
    for (int bit = 0; bit < bits_in_mask; bit++){
        int bitIndex = get_ls1b_index(mask);

        if (index & (1<<bit))
            set_bit(occupancies, bitIndex);
        pop_bit(mask, bitIndex);
    }
    return occupancies;
}

void init_rook_magics(){
    for (int square = 0; square < 64; square++){
        int indexLimit = 1 << count_bits(rook_relevant_occupancies[square]);
        for (int index = 0; index < indexLimit; index++){
            U64 occupancies = set_occupancy(index, rook_relevant_occupancies[square]);
            int magic_index = (int)((rook_magic_numbers[square] * occupancies) >> (64-rook_relevent_occupancy_count[square]));
            rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancies);
        }
    }
}

void init_bishop_magics(){
    for (int square = 0; square < 64; square++){
        int indexLimit = 1 << count_bits(bishop_relevant_occupancies[square]);
        for (int index = 0; index < indexLimit; index++){
            U64 occupancies = set_occupancy(index, bishop_relevant_occupancies[square]);
            int magic_index = (int)((bishop_magic_numbers[square] * occupancies) >> (64-bishop_relevent_occupancy_count[square]));
            bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancies);
        }
    }
}



void init_bitboards(){
    //init_knight_mask();
    //init_pawn_mask();
    //init_king_mask();

    init_rook_relevant_occupancies();
    init_bishop_relevant_occupancies();

    init_rook_magics();
    init_bishop_magics();
}
