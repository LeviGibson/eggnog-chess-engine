var flip = 0;
var sqrSize = 36;
var positionSetup = 0;
var whitePrefix = "White";
var blackPrefix = "Black";
var graphSuffix = "png";
var realColor = "";
var useMarkers = 0;
var noCounts = 0;
var startMessage = 'Interactive diagram</a>: you can move the pieces!</b>';
var markerDir = "/membergraphics/MSelven-chess/";
var hand  = new Array();
var board = new Array();
var shadow = new Array();
var moves = new Array();
var imag = new Array();
var ids  = new Array();
var names = new Array();
var startSqr = new Array();
var xStep = new Array();
var yStep = new Array();
var vector = new Array();
var ranges = new Array();
var flags = new Array();
var skip  = new Array();
var first = new Array();
var sanMoves = new Array();
var diagDesc = new Array();
var choice = new Array();
var royal = new Array();
var royalness = new Array();
var royalCount = new Array();
var strength = new Array();
var imitator = new Array();
var promoDepth = new Array();
var asym = new Array();
var oldImag = '';
var oddShade = 0;
var clocks = 0;
var chatID = 0;
var rank1 = 1;
var timer = 0;
var sequel = 0;
var sequel2 = 0;
var pacify = 0;
var pacoMode = 0;
var extinction = 0;
var enforceRules = 0;
var activePlayer = 0;
var CustomPromotion;
var useAI = new Array();
if(typeof(scriptSeen) == 'undefined') { scriptSeen = 7; timer = setTimeout("Init();", 1000); }

function CreateBoard(bnr, err) {
  tab = "";
  for(i=0; i<ranks; i++) {
    row = "";
    if(err) { board[i] = new Array(); start[i] = new Array(); shadow[i] = new Array(); }
    if(coords >= 0) row += '<td id="' + bnr + 'r' + i + '" border="0" style="text-align:right">' + (ranks - i - 1 + rank1) + '&nbsp;</td>';
    for(j=0; j<files; j++) {
      if(err) board[i][j] = 0;
      h = ranks - 1 - i;
      var s = OwnShade(h, j);
      row = row + '<td id="' + bnr + 'y' + h + 'y' + j + '" style="width:' + sqrSize + 'px;height:' + sqrSize + 
            'px;border:thin solid black;font-size:xx-large;background-repeat:no-repeat;background-position: center center"' +
            ' onmousedown="Down(' + bnr + ',' + j + ',' + h + ',event)" onmouseup="Up(' + bnr + ',' + j + ',' + h +
            ')" ontouch="Touch(' + bnr + ',' + j + ',' + h +
            ')" onmouseover="Hover(event' +
            ')" ondragover="PreDrop(event)" ondrop="Drop(event)" ondragend="Relay()" valign="center" align="center"></td>';
    }
    if(clocks) {
      if(i == 0) row = row + '<td class="clock" id="topclock" width="' + clocks*sqrSize + '">0:00</td>';
      if(i == 1) row = row + '<td id="topscore" align="right" width="' + clocks*sqrSize + '"></td>';
      if(i == ranks-2) row = row + '<td id="bottomscore" align="right" width="' + clocks*sqrSize + '"></td>';
      if(i == ranks-1) row = row + '<td class="clock" id="bottomclock" width="' + clocks*sqrSize + '">0:00</td>';
      clockTimes[0] = clockTimes[1] = 600;
    }
    tab = tab + '<tr>' + row + '</tr>';
  }
  if(coords >= 0) {
    tab += '<tr><td></td>';
    for(j=0; j<files; j++) tab += '<td id="' + bnr + 'f' + j + '" style="text-align:center">' + String.fromCharCode(97+j) + '</td>';
    tab += '</tr>';
  }
  document.getElementById("board" + bnr).innerHTML = tab;
}

var buttonBar = 
    '<input type="button" onclick="FlipView()" value="Flip View">\n' +
    '<input class="step" type="button" onclick="Seek(0)" value="|<">\n' +
    '<input class="step" type="button" onclick="Seek(10001)" value="<">\n' +
    '<input class="step" type="button" onclick="Seek(10002)" value=">">\n' +
    '<input class="step" type="button" onclick="Seek(10003)" value=">>">\n' +
    '<input class="step" type="button" onclick="Seek(10000)" value=">|">\n';

var diagList = new Array();

function Init() {
  err = 3; castleFlip = 0; ids[0] = '?';
  if(document.getElementById('board') == null) {
    var dl = document.getElementsByClassName('idiagram');
    var i, nrDiags = dl.length;
    for(i=0; i<nrDiags; i++) diagList[i] = dl[i];
    var diag = document.getElementById('diagram');
    if(diag != null) diagList[i++] = diag;
    if(i == 0) return;  // page contains no diagram
    while(i-- > 0) {
      diagDesc[i] = diagList[i].innerHTML;
      if(i == nrDiags && nrDiags > 0) diagList[i].parentElement.style.cssFloat = 'none';
      err = ParseGameDef(i); // try to fetch game definition from HTML body
      if(err == 1) {
        diagList[i].innerHTML = '<h1>Cannot make a diagram with ' + nType + ' pieces on an ' + files + 'x' + ranks + '  board!</h1>';
        continue;
      }
      CreateAnchors(diagList[i], i); CreateBoard(i, err);
      BetzaCompile();
      Display();
    }
  }
  // the following is only for navigation on pages with single diagram
  for(i=0; i<ranks; i++) for(j=0; j<ranks; j++) start[i][j] = board[i][j];
  for(i=0; i<=2*nType; i++) startHand[i] = hand[i];
  var bar = document.getElementById('buttonbar');
  if(bar != null) bar.innerHTML = buttonBar;
  if(typeof(CreateChat) == 'function') CreateChat(); // optional chat on page
  if(typeof(WeirdPromotion) == 'function') OwnPromotion = WeirdPromotion; // legacy promotion overruler
  if(clocks) ClockTick();
}

function OwnShade(x, y) { // board checkering
  if(shady && typeof(Shade) == 'function') return Shade(x,y);
  return ((x&1) == (y&1));
}

var OwnPromotion = function(x1, y1, x2, y2, promo, ep1, ep2) {
  if(typeof(CustomPromotion) == 'function') {
    var piece = (x1 < 100 ? board[y1][x1] : 511);
    CustomPromotion(piece & 1024, piece & 511, x1, y1, x2, y2, promo, board[y2][x2] & 2047, ep1, ep2);
  }
}

function DefaultMove(s) { // moves of some commonly used pieces
  if(s == 'pawn') return "ifmnDfmWfceF"; // FIDE
  if(s == 'king') return (files > 8 ? "KisO3" : "KisO2");
  if(s == 'knight') return "N";
  if(s == 'bishop') return "B";
  if(s == 'rook') return "R";
  if(s == 'queen') return "Q";
  if(s == 'cardinal') return "BN"; // Capablanca
  if(s == 'archbishop') return "BN";
  if(s == 'chancellor') return "RN";
  if(s == 'marshall') return "RN";
  if(s == 'amazon') return "QN";
  if(s == 'lion') return "KNADcaKcabKmabK"; // Chu Shogi
  if(s == 'mao') return "mafsW"; // Xiangqi
  if(s == 'horse') return "mafsW";
  if(s == 'elephant') return "mafF";
  if(s == 'cannon') return "mRcpR";
  if(s == 'pao') return "mRcpR";
  if(s == 'advisor') return "F";
  if(s == 'lance') return "fR"; // Shogi
  if(s == 'copper') return "vWfF";
  if(s == 'silver') return "FfW";
  if(s == 'silvergeneral') return "FfW";
  if(s == 'gold') return "WfF";
  if(s == 'goldgeneral') return "WfF";
  if(s == 'man') return "K"; // common fairies
  if(s == 'commoner') return "K";
  if(s == 'ferz') return "F";
  if(s == 'camel') return "C";
  if(s == 'zebra') return "Z";
  if(s == 'nightrider') return "N0";
  if(s == 'grasshopper') return "gQ";
  if(s == 'griffon') return "yafsF";
  if(s == 'gryphon') return "yafsF";
  if(s == 'wizard') return "FC"; // Omega
  if(s == 'champion') return "WAD";
  if(s == 'falcon') return "mafmafsKmafsmafKmaflmafrKmafrmaflK";
  return "O";
}

function PutOne(f, r, piece) {
  if((piece & 511) == 250) shadow[r][f] = piece = 250; // holes
  board[r][f] = piece;
}

function PutInner(f, r, piece, sym) {
  var color = piece & 1024;
  PutOne(f, r, piece);
  if(color == 0) { // add black partner of white piece
    if(sym == 'rotate') PutOne(files-1-f, ranks-1-r, piece|1024);
    if(sym == 'mirror') PutOne(f,         ranks-1-r, piece|1024);
  }
}

function PutPiece(sqr, piece, sym) {
  var a = "a".charCodeAt(0);
  var f = sqr.charCodeAt(0) - a;
  var r = parseInt(sqr.substring(1,3)) - rank1;
  var bounds = sqr.split('-');
  if(bounds.length < 2) { PutInner(f, r, piece, sym); return; } // just one
  var e = bounds[1].charCodeAt(0) - a;
  var s = parseInt(bounds[1].substring(1,3)) - rank1;
  for(var i=r; i<=s; i++)  for(var k=f; k<=e; k++) PutInner(k, i, piece, sym);
}

var initDone = 0;

function SetPromoChoice() {
  for(i=1; i<=nType; i++) {
    k = promoChoice.split(ids[i]);
    if(k.length > 1) {
      choice[i] = (k[0].length < 1 ? 1 : k[0][k[0].length-1] == '!' ? 2 : k[0][k[0].length-1] == '*' ? 3 : 1);
      k = parseInt(k[1]); promoDepth[i] = (k ? k : promoZone);
    } else choice[i] = (promoChoice == '');
  }
}

function ParseGameDef(dnr) {
  var n = 0, m = 0;
  zonal =  typeof(BadZone) == 'function'; // user-supplied function for indicating no-go area
  active = dnr; hx1= -1; hx2 = -1; movedPiece = 1024; movedX = -1; movedY = -1; ai = rank1 = shady = 1;
  if(typeof(Setup) != 'function') { // HTML header contains no script; set defaults
    ranks = 8; files = 8; coords = -1; holdingsType = autoGate = thresholdPiece = 0; extinction = 3; var sym = 'mirror';
    promoZone = maxPromote = 1; promoOffset = 0; promoChoice = 'QNRB'; shuffle = ''; satellite = 'piece';
    for(var i=0; i<512; i++) royal[i] = royalness[i] = 0; haveRoyal = stalemate = 0;
    useMarkers = 0; light = "#FFFFFF"; dark = "#E0E0E0"; // mild checkering
    graphDir = "/membergraphics/MSinteractive-diagrams/"; graphSuffix = "pgn"; whitePrefix = "w"; blackPrefix = "b";
  }
  // get definition
  var text = diagDesc[dnr];
  if(text == '') return 2; // no game definition in HTML body
  gameDef = text.split('\n');
  for(var i=0; i<gameDef.length; i++) {
    for(j=0; gameDef[i][j] == ' '; j++); gameDef[i] = gameDef[i].substring(j); // strip leading spaces
    var words = gameDef[i].split('<br>');
    if(words.length > 1) gameDef[i] = words[0]; // strip trailing <br> (for phpBB)
    words = gameDef[i].split('=');
    for(j=2; j<words.length; j++) words[1] += '=' + words[j];
    if(words.length > 1) { // opportunity to overrule defaults
      if(words[0] == 'files') files = parseInt(words[1]); else
      if(words[0] == 'ranks') ranks = parseInt(words[1]); else
      if(words[0] == 'holdingsType') holdingsType = parseInt(words[1]); else
      if(words[0] == 'promoZone') promoZone = parseInt(words[1]); else
      if(words[0] == 'maxPromote') maxPromote = parseInt(words[1]); else
      if(words[0] == 'promoChoice') promoChoice = words[1]; else
      if(words[0] == 'promoOffset') promoOffset = parseInt(words[1]); else
      if(words[0] == 'graphicsDir') graphDir = words[1]; else
      if(words[0] == 'lightShade') light = words[1]; else
      if(words[0] == 'darkShade') dark = words[1]; else
      if(words[0] == 'startShade') { realColor = dark; dark = words[1]; } else
      if(words[0] == 'shady') shady = parseInt(words[1]); else
      if(words[0] == 'zonal') zonal = parseInt(words[1]); else
      if(words[0] == 'squareSize') sqrSize = words[1]; else
      if(words[0] == 'symmetry') sym = words[1]; else
      if(words[0] == 'graphicsType') graphSuffix = words[1]; else
      if(words[0] == 'whitePrefix') whitePrefix = words[1]; else
      if(words[0] == 'blackPrefix') blackPrefix = words[1]; else
      if(words[0] == 'satellite') satellite = words[1]; else
      if(words[0] == 'royal') royal[j=parseInt(words[1])] = 1, royalness[j] |= 3<<10, haveRoyal = 1; else
      if(words[0] == 'iron') royalness[parseInt(words[1])] |= 3<<4; else
      if(words[0] == 'counterStrike') royalness[parseInt(words[1])] |= 3<<6; else
      if(words[0] == 'protected') royalness[parseInt(words[1])] |= 3<<8; else
      if(words[0] == 'antiTrade') { j = parseInt(words[1]); if(n) royalness[n] = 3<<16+2*(j&3); else royalness[j] |= 7<<13; } else
      if(words[0] == 'extinction') extinction = parseInt(words[1]); else
      if(words[0] == 'tradeThreshold') thresholdPiece = parseInt(words[1]); else
      if(words[0] == 'useMarkers') useMarkers = parseInt(words[1]); else
      if(words[0] == 'castleFlip') castleFlip = parseInt(words[1]); else
      if(words[0] == 'firstRank') coords = rank1 =  parseInt(words[1]); else
      if(words[0] == 'enableAI') ai = parseInt(words[1]); else
      if(words[0] == 'autoGate') autoGate = parseInt(words[1]); else
      if(words[0] == 'shuffle') shuffle = words[1]; else
      if(words[0] == 'stalemate') stalemate = (words[1] == 'draw' ? 0 : -20000); else
      if(words[0] == 'baring') { j = parseInt(words[1]); if(j) royalness[j] |= 15; else for(j=1; j<250; j++) if(!royal[j]) royalness[j] |= 15; }
    } else {
      var parts = gameDef[i].split(':'), color = 0, j;
      if(parts.length < 5) continue;
      if(ranks > initDone) {
        for(j=initDone; j<ranks; j++) { board[j] = new Array(); start[j] = new Array(); shadow[j] = new Array(); }
        ClearBoard(); initDone = ranks;
      }
      n++; if(!m++) ClearBoard(); // is piece definition
      names[n] = parts[0];
      ids[n]   = parts[1];
      moves[n] = parts[2];
      imag[n]  = parts[3];
      startSqr[n] = (parts[4].split(',,'))[0];
      hand[n]  = (parts.length > 5 ? parts[5] : '0');
      if(names[n].length > 0) { // derive missing stuff from name
        if(imag[n].length == 0) imag[n] = names[n]; // case still conserved!
        names[n] = names[n].toLowerCase();
        if(moves[n].length == 0) moves[n] = DefaultMove(names[n]);
        if(ids[n].length == 0) ids[n] = (names[n].toUpperCase())[0];
      }
      if(imag[n].length == 0) { n--; continue; } // image is mandatory
      if(graphSuffix != '') imag[n]  +=  '.' + graphSuffix;
      var nn = n; if(names[n] == 'hole') { nn = 250; n--; } // holes in board
      var squares = parts[4].split(',');
      if(parts[4] == 0) continue;
      for(j=0; j<squares.length; j++) {
        if(squares[j].length < 2) { color = 1024; continue; }
        PutPiece(squares[j], nn + color, sym);
      }
    }
  }
  nType = n; if(!haveRoyal) royal[nType] = 1, royalness[nType] |= 3<<10, royalness[nType]&= ~15;
  if(nType < 1 || files < 1 || ranks < 1) return 1;
  for(i=1; i<=nType; i++) {
    var k, s = hand[i].split(',');
    hand[i] = parseInt(s[0]);
    hand[i+nType] = (s.length == 1 ? hand[i] : parseInt(s[1]));
//    if(royal[i]) royalness[i] &= ~15; // erase baring on royals
  }
  SetPromoChoice();
  castleRank = -1;
  for(i=0; i<ranks; i++) for(j=0; j<files; j++) {
    var pc = start[i][j] = board[i][j];
    if(pc && royal[pc&511] && moves[pc&511].split('O').length > 1) castleRank = i, aSide = hSide = j;
  }
  if(castleRank >= 0) { // figure out castling partners (assume same for black)
    while(aSide >= 0 && board[castleRank][aSide] != 250) aSide--; while(board[castleRank][++aSide] == 0);
    while(hSide < files && board[castleRank][hSide] != 250) hSide++; while(board[castleRank][--hSide] == 0);
  }
  if(castleRank >= ranks/2) castleRank = ranks - 1 - castleRank;
  for(i=0; i<=2*nType; i++) startHand[i] = hand[i];
  if(pacoMode) for(i=1; i<7; i++) for(j=1; j<7; j++) ids[64+8*i+j] = ids[i] + ids[j];   // combis
  if(screen.colorDepth <= 16) useMarkers = 2, realColor = '#E0E0E0', light = '#FFFFFF'; // assume grey-scale display
  symmetry = sym; haveShuffle = 0;
}

function ShuffleOne(color, shuffle) {
  if(shuffle == '') return;
  var counts = new Array(), square = new Array();
  var king = 0, rook = 0, sq = 0;
  for(r=1; r<=nType; r++) {
    if(royal[r] && moves[r].split('O').length > 1) king = r;       // find piece that castles
    counts[r] = 0;
  }
  for(var r=0; r<ranks; r++) for(var f=0; f<files; f++) {
    var piece = board[r][f];
    if(piece && (piece & 1024) == color) {
      piece &= 511;
      var p = shuffle.split(ids[piece]);
      if(p.length != 2) continue;                      // not in shuffle
      counts[piece]++;                                 // count pieces
      square[sq++] = 2048*r + f;                       // remember squares
      if(piece == king) rook = board[r][0] & 511;
      if(rook != (board[r][files-1] & 511)) rook = 0;
    }
  }
  for(r=1; r<=nType; r++) { // pass 1: color bounds
    var k = shuffle.split(ids[r]);
    if(k.length != 2 || counts[r] < 2) continue; // not a pair to shuffle
    var c = k[0][k[0].length-1];                 // prefix
    if(c != '!' && c != ':') continue; // not a pair of color-balanced
    var shade = -1, last, n = sq;
    while(counts[r] && n) {
      var i = Math.floor(Math.random()*n);
      var s = square[i]; square[i] = square[--n]; square[n] = s;
      if(shade == (2049*s & 2048)) continue; // same color as previous: reject
      if(c == ':' && counts[r] & 1 && (last + s & 2047) != files-1) continue; // not symmetric, reject
      square[n] = square[--sq]; counts[r]--; n = sq;
      PutInner(s&2047, s>>11, r + color, symmetry);
      shade = 2049*s & 2048; last = s;
  } }
  for(r=1; r<=nType; r++) {
    while(r != king && counts[r] > (r == rook ? 2 : 0)) { // pass 2: place non-castling
      var i = Math.floor(Math.random()*sq);
      var s = square[i]; square[i] = square[--sq]; counts[r]--;
      PutInner(s&2047, s>>11, r + color, symmetry);
  } }
//statuss.innerHTML += 'Shuffle; left='+sq+' squares K='+king+' R='+rook+'<br>';
  if(king > 0 && rook > 0 && counts[rook] == 2 && counts[king] == 1) {
    k = (square[1] > square[0] ? 1 : 0);
    if(square[k] > square[2]) k = (square[1-k] > square[2] ? 1-k : 2);
    for(f=0; f<3; f++) PutInner(square[f]&2047, square[f]>>11, (f == k ? king : rook) + color, symmetry);
  }
}

var lastShuffle = new Array();
var haveShuffle = 0;

function Shuffle(same) {
  if(same) {
    if(haveShuffle)
      for(var r=0; r<ranks; r++) for(var f=0; f<files; f++) board[r][f] = lastShuffle[r][f];
    return;
  }
  var parts = shuffle.split(',');
  for(var i=0; i<parts.length; i++) {
    ShuffleOne(0, parts[i]);
    if(symmetry == 'none') ShuffleOne(1024, parts[i]);
  }
  haveShuffle = 1;
  for(var r=0; r<ranks; r++) { lastShuffle[r] = new Array(); for(var f=0; f<files; f++) lastShuffle[r][f] = board[r][f]; }
}

function Legend(color, betza, text) {
  var img = (color == "00FFFF" ? ' style="background-color:#C0C0FF"' : '') + '><img src="' + markerDir + 'mark' + color + '.png" height="21" width="21">';
  if(!useMarkers) img = ' height="21" width="21" style="background-color:#' + color + '">';
  return '<tr><td' + img + '</td><td><b>' + betza + '</b></td><td>' + text + '</td></tr>';
}

function ColorLegend() {
  return '<br><table cellpadding="0" border="1" style="border-collapse: collapse">' + 
    Legend('FFFF00', 'mc', 'Move or capture') +
    Legend('FFC000', 'mc', 'Move or capture (unblockable jump)') +
    (modes & 1 ? Legend('C0C000', 'imc', 'Move or capture (virgin piece only)') : '') +
    Legend('00FF00', 'm', 'Move only') +
    Legend('00C000', 'im', 'Move only (virgin piece only)') +
    Legend('FF0000', 'c', 'Capture only') +
    (modes & 2 ? Legend('0000FF', 'd', 'Capture own piece') : '') +
    (modes & 4 ? Legend('00FFFF', 'ca', 'Locust capture (hover/touch for sequel)') : '') +
    (modes & 8 ? Legend('00A000', 'O', 'Castling destination of King') : '') +
    Legend('A0A0A0', 'm,c', 'Forbidden move of royal into check') +
    (modes & 16 ? '<tr><td style="text-align:center;background-color:#' + (useMarkers ? 'C0C0C0' : 'FFFFFF') + '" height="21">' +
    (useMarkers? '' : '<b>?</b>') +
    '</td><td><b>p,g</b></td><td>Hop target (hover/touch for actual move)</td></tr>' : '') +
    '<td colspan="3" onclick="useMarkers = 1 - useMarkers;BetzaCompile();">&nbsp;Click on piece to toggle virginity on/off</td></tr>' +
    '</table>';                
}

function CreateAnchors(diag, dnr) {
  var table = document.getElementById(satellite + 'Table');
  var menu = document.getElementById(satellite + 'Colors');
  var aiMessage = '<u onclick="OpenDiv(\'bar' + dnr + '\'); useAI['+dnr+'] = !useAI['+dnr+'];" style="color:blue">Play it!</u>';
  var aiBar =   '<div id="bar' + dnr + '" style="display:none"><p><i style="background:#C0C0FF">The AI will now respond to your moves!</i> ' + 
                '(&nbsp;<b onclick="Plies(1);" style="background:#E0E0E0;border:1px solid grey;border-radius:3px"> + </b>&nbsp;<span id="ply' + dnr + '">2 ply</span>&nbsp;' +
                '<b onclick="Plies(-1);" style="background:#E0E0E0;border:1px solid grey;border-radius:3px"> - </b>&nbsp;)</p>' +
                '<input type="button" onclick="NewGame()" value="Restart">\n' + buttonBar + 
                '<input type="button" onclick="AiMove()" value="Move">' +
                '<p id="aiGame' + dnr + '" style="width:400px"></p></div>';
  var pieceInfo = "";
  if(table == null) table = document.getElementById('pieceTable' + dnr);
  else table.id = 'pieceTable' + dnr;
  if(table == null) // add collapsed piece-info display below the diagram, if not already present elsewhere
    pieceInfo = '<p>Click <u onclick="if(OpenDiv(\'legend' + dnr + '\'))OpenDiv(\'description' + dnr + '\');" style="color:blue">' +
                'here</u> to open/close a piece overview. ' + (ai ? aiMessage : '') + '</p>' + aiBar +
                '<div id="description' + dnr + '" style="display:none"><table id="piecedesc' + dnr + '" border="1"><tr>' +
                '<td>Info for selected board piece</td></tr></table>' +
                (!menu ? '<div id="' + satellite + 'Colors"></div>' : '') +
                '</div>' + 
                '<div id="printversion' + dnr + '"></div>' +
                '<div id="legend' + dnr + '" style="display:none">' +
                '<table id="pieceTable' + dnr + '" border="1"></table></div>';
  diag.innerHTML = '<p id="message' + dnr + '"><b style="background:red"><a href="http://www.chessvariants.com/invention/interactive-diagrams">' +
                   startMessage + '</p>' +
                   '<table id="board' + dnr + '" cellpadding="0" style="border-collapse:collapse"></table>' +
                   (pieceInfo != '' ? pieceInfo : ai == 2 ? '&nbsp;&nbsp;&nbsp;&nbsp;<i>This diagram plays chess!</i> ' + aiMessage + aiBar : '');
  diag.style.display = 'inherit'; // description div would normally start invisible
}

function ClearBoard() {
  for(var i=0; i<ranks; i++) for(var j=0; j<files; j++) board[i][j] = shadow[i][j] = 0;
}

function ShowMessage(msg, color) {
  if(color != undefined) msg = '<span style="background:' + color + '">' + msg + '</span>';
  document.getElementById('message' + active).innerHTML = msg;
}

function AppendMessage(msg) {
  document.getElementById('message' + active).innerHTML += msg;
}

var hx1 = -1;
var hy1 = 0;
var hx2 = -1;
var hy2 = 0;

var source_cell;

function image_dragover(event) {
  if (source_cell == event.target.parentElement.id)
    event.target.src = 'empty.png';
}

function image_dragstart(event) {
  event.target.style.opacity=1.0;
  source_cell = event.target.parentElement.id
}

function Display() {
  var p;
  for(i=0; i<ranks; i++) for(j=0; j<files; j++) {
    if(flip) {
      x = files - 1 - j;
      y = ranks - 1 - i;
    } else {
      x = j; y = i;
    }
    cell = active + "y" + y + "y" + x;
    board[i][j] &= 2048 - 1; // erase markers
    piece = board[i][j] & ~512;
    if(piece > 1024) {
      piece -= 1024; color = blackPrefix;
    } else {
      color = whitePrefix;
    }
    if(piece > 0 && piece != 250) {
//      image = '<img ondragover="Parent(event)" src="' + MakeImage(piece, color) + '" style="vertical-align:top">';
      image = 'url("' + MakeImage(piece, color) + '")';
    } else {
      image = "";
    }
    //document.getElementById(cell).style.backgroundImage = image;
    document.getElementById(cell).innerHTML = '<img ondragover="image_dragover(event)" ondragstart="image_dragstart(event)" ' + image.replace('url(', 'src=').replace(')', '>');
    if(piece == 250) Highlight(j, i, "#000000"); else
    if(j == hx1 && i == hy1 || j == hx2 && i == hy2) Highlight(j, i, "#80FF80"); else Highlight(j, i, 0);
  }
  if(document.getElementById('pieceTable' + active) != null) {
    if(oldImag != graphDir + whitePrefix + blackPrefix) BetzaCompile(); // switched images, adapt table
    for(i=1; i<=nType; i++) {
      w = hand[i]; if(w == 0) w = "-";
      b = hand[i+nType]; if(b == 0) b = "-";
      p = document.getElementById(active + 'w' + i);
      if(p != null) p.innerHTML = w;
      p = document.getElementById(active + 'b' + i);
      if(p != null) p.innerHTML = b;
    }
  }
  if(realColor != '' && realColor != dark && !touched) { if(light == dark) light = realColor; dark = realColor; touched = -1; }
  oddShade = 0;
}



function FlipView() {
  flip = !flip;
  Display(); if(clocks) DisplayClock(0), DisplayClock(1), DisplayScore(displayed), DisplayScore(displayed-1);
  for(i=0; i<files; i++) document.getElementById(active + 'f' + i).innerHTML = String.fromCharCode(flip ? 96 + files - i : 97 + i);
  for(i=0; i<ranks; i++) document.getElementById(active + 'r' + i).innerHTML = (flip ? rank1 + i : ranks - i + rank1 - 1) + '&nbsp;';
}

var zonal = 0, san = 0, sanX, sanY, movedPiece, movedX, movedY, captPiece, imitatedType;

function Highlight(x, y, color) {
  var old = 0, markers = useMarkers && x < 100;
  if(x < 100 && (board[y][x]&511) == 250) color = "#000000";
  if(checkFlag || color && zonal && x < 100 && board[y][x] != 250 && BadZone(x, y, piece & 512-1, piece & 1024)) return;
  if(san) { if(x == sanX && y == sanY) san |= 1; return; }
  if(pacify && color == '#FF0000') return;
  if(x < 100 && color != 0)  old = board[y][x] & (1<<30), board[y][x] |= (1<<30);
  if(color == 0) {
    if(x < 100) {
      color = (OwnShade(x,y) ? (oddShade && x & 1 ? oddShade : dark) : light);
      if(shadow[y][x] == 250) color = "#D0C0B8";
    } else color = "#FFFFFF";
    markers = 0;
  }
  else if(color == "#80FF80" || color == "#000000" || color == "#FF8080" || color == "#C0C0C0") markers = 0; // move highlights & holes
  else if(x < 100 && blockFlag && !royal[board[y][x] & 512-1]) color = "#A0A0A0";
  if(flip && x < 100) {
    x = files - 1 - x;
    y = ranks - 1 - y;
  }
  var suppress = (touched && old && color == "#00FFFF");
  if((color == "#C0C0C0" || color == "#00FFFF" && touched) && !useMarkers)
    document.getElementById(active + 'y' + y + 'y' + x).innerHTML = '<span style="font-size:' + (3*sqrSize>>2) + 'px">?</span>'; else
  if(markers && !suppress)
    document.getElementById(active + 'y' + y + 'y' + x).innerHTML =
      '<img style="display:block" onmouseover="Parent(event)" ontouch="Parent(event)" src="' + markerDir + 'mark' + color.substring(1) + '.png">';
  if(markers ? touched && color == "#00FFFF" && (color = "#C0C0FF") : !suppress)
    document.getElementById(active + 'y' + y + 'y' + x).style.backgroundColor = color;
}

function Lowlight() {
  if(hx1 >= 0) Highlight(hx1, hy1, 0);
  if(hx2 >= 0) Highlight(hx2, hy2, 0);
  hx1 = -1; hx2 = -1;
}

function Promoted(n) {
  n = n & 511;
  if(n > maxPromote) return n;
  n += promoOffset;
  if(n > nType) n = promoOffset + 1;
  return n;
}

function Promotion(x, y, f, r) {
  if(positionSetup) return 0; // no promotions in setup mode
  var p = board[y][x] & (2048 - 1 - 512);
  if((p & 511) > maxPromote) return 0; // not promotable
  var v = board[r][f] & 2047;
  if(p < 1024) { r = ranks - 1 - r; y = ranks - 1 - y; } else p -= 1024;
  if((p & 511) == 1 && (r == 0 && promoZone > 0)) return 2; // Pawn must promote on last rank
  if(y < promoZone && holdingsType == 0) return (v != 0 || !promoOffset);  // was already in; only on capture
  return (r < promoZone || y < promoZone);
}

function Hold(piece, hType) {
  piece &= 2048 - 1 - 512;
  if(piece == 0 || hType == 0) return;
  if(hType < 0) piece ^= 1024; // toggle color
  if(hType == -1 && promoOffset && (piece & 511) > promoOffset) piece -= promoOffset; // demote
  if(piece > 1024) piece += nType - 1024;
  hand[piece]++;
}

function MakeMove(x1, y1, x2, y2, epx, epy, promo) {
  flagX = -1; flagY = -1; // clear old e.p. rights
  if(x1 < 100) {
    piece = board[y1][x1]; if(sequel2) piece = sequel2, sequel2 = 0; else
    board[y1][x1] = shadow[y1][x1]; Hold(board[y2][x2], holdingsType);
  } else {
    piece = y1; if(piece > nType) piece += 1024 - nType;
    hand[y1]--;
    victim = board[y2][x2];
    Hold(victim, ((victim ^ piece) & 1024) == 0 ? 1 : holdingsType); // always allow swapping of own pieces
    if(positionSetup > 1) {
      if(positionSetup > 2) board[ranks-1-y2][files-1-x2] = piece ^ 1024;
      if(positionSetup & 2) board[ranks-1-y2][x2] = piece ^ 1024;
      if(positionSetup == 3) board[y2][files-1-x2] = piece;
    }
  }
  movedPiece = piece; movedX = x2; movedY = y2;  // remember for AI 'move' button
  captPiece = board[y2][x2];
  if(!imitator[piece&511]) imitatedType = piece; // avoid recursive imitation
  if(epx >= 0) { // side effect
    if(epx == 100) { // specifies creation of rights
      flagX = x1; flagY = y1; lastX = x2; lastY = y2; // set e.p. rights
    } else {
      victim = board[epy][epx];
      var drop = shadow[epy][epx];
      if((victim & 2047) == 0) drop = board[y2][x2]; else // unload regular victim on empty e.p. square
      if(((victim ^ piece) & 1024) == 0 && (victim & 511) <= nType) { // locust capture of own piece
        x1 = (x1 > x2 ? x2 + 1 : x2 - 1), board[y2][x1] = victim | (victim & 511 ? 512 : 0);
      } else Hold(victim, holdingsType); // locust capture
      board[epy][epx] = drop;
    }
  }
  if(promo & 256) board[epy][epx] = promo - 256; else
  if(promo != 0) piece = (promo == 1023 ? piece + promoOffset : promo & ~1024 | piece & 1024);
  board[y2][x2] = piece | (piece & 511 ? 512 : 0);
  CountRoyals();
}

function Move(x1, y1, x2, y2, epx, epy, oldPromo) {
  if(typeof(WeirdPromotion) == 'function')
    promo = WeirdPromotion(x1, y1, x2, y2, oldPromo, epx, epy); // for variants with custom promotion
  else promo = oldPromo;
  if(oldPromo == 1022 && (promo & 2048) == 0) return 1; // merge request rejected if not accepted through bit 11
  if(promo == 1022) return 1; // rejection code for custom move vetting in WeirdPromotion
  promo &= 2047;
  sanMoves[displayed] = ToSAN(x1, y1, x2, y2, epx, epy, promo);
  var ptr = document.getElementById('sanmove');
  if(ptr != null) ptr.innerHTML = sanMoves[displayed];
  AddMove(x1, y1, x2, y2, epx, epy, promo);
  MakeMove(x1, y1, x2, y2, epx, epy, promo);
  hx1 = x1; hy1 = y1; hx2 = x2; hy2 = y2; // highlights
  Display();
  if(epx >= 0 && epx < 100) Highlight(epx, epy, "#FF8080");
  ptr = document.getElementById('game');
  if(!ptr) ptr = document.getElementById('aiGame' + active);
  if(ptr != null) ptr.innerHTML = FlushSAN();
  return 0;
}

var xx = -1;
var yy = -1;
var ex = -1;
var ey = -1;
var px = -1;
var py = -1;
var flagX = -1;
var flagY = -1;
var lastX = 0;
var lastY = 0;
var phase;

var curCell;
var obstacle = 0, emulateHover = 0;

function Interpose(s) {
  if(touched <= 0) return;
  var t = touched;
  if(s.length < 5) return;
  var a = s.split('y');
  if(a.length != 3) return;
  if(active != a[0]) return;
  if(board[a[1]][a[2]] & (1<<28)) {
    touched = -1; obstacle = 1;
    ShowMoves2(active, t, a[2], a[1]);
  } else if(obstacle) {
    touched = -1; obstacle = 0;
    ShowMoves2(active, t, -1, 0);
  }
}

function Hover(ev) {
  emulateHover = 0;
  Interpose(ev.target.id);
}

function Relay() { // catches mouse-up after drag in source. (destination might be eclipsed)
  var coords = curCell.split('y');
  if(coords.length != 3) return;
  Up(parseInt(coords[0]), parseInt(coords[2]), parseInt(coords[1]));
}

function PreDrop(ev) { // keeps track of cell we are in
  ev.preventDefault();
  s = ev.target.id; if(s.length > 2) curCell = s;
}

function Parent(ev) { // keeps track of cell we are in, when image hides cell
  s = ev.target.parentElement.id; if(s.length > 2) curCell = s, Interpose(s);
}

function Drop(ev) { // needed to prevent browser follows link to dragged image
  ev.preventDefault();
}

function ShogiProm(yes) {
  var prom = (board[yy][xx] & 2048-1) + yes*promoOffset;
  if((prom & 511) > nType) prom = promoOffset + 1;
  Move(xx, yy, px, py, ex, ey, prom);
  Display();
  ShowMessage("Next move:");
  xx = -1; px = -1; ex = -1;
}

function SwitchDiag(n) {
  if(n == active) return;
  ParseGameDef(n);
  if(realColor != '' && realColor != dark) { if(light == dark) light = realColor; dark = realColor; }
  BetzaCompile();
  xx = -1; ex = -1; px = -1;
  Display();
}

function Click(x, y) {
  var promoType = 0, moved = 0;
  if(touched > 0 && x == files >> 1 && y == ranks-1 >> 1) { ShowMoves2(active, touched ^ 512, -1, 0); return; }
  if(emulateHover) return;
  if(touched) { if(touched < 0) Display(), touched = 0; else { ShowMoves2(active, 0, -1, 0); return; } }
  if(displayed != gameLen) {
    if(useAI[active]) gameLen = displayed; else {
      ShowMessage('Position not current; press ">|" first', 'red');
      return;
    }
  }
  if(x < 100) {
    if(px >= 0) { // board click instead of promo choice: abort promotion move
      for(i=1; i<=2*nType; i++) Highlight(100, i, 0);
      Lowlight(); Display(); ShowMessage('Next move:');
      xx = -1; px = -1; ex = -1;
    }
    if(flip) {
      x = files - 1 - x;
      y = ranks - 1 - y;
    }
    piece = board[y][x];
    if(positionSetup && xx < 0 && !piece && hx1 == 100) { xx = hx1; yy = hy1; } // multiple to-clicks
  } else {
    piece = (y > nType ? y - nType : y);
    if(px >= 0) { // this is the promotion choice we are waiting for
      if(promoChoice.length) {
        y = piece + (board[yy][xx] & 1024 ? nType : 0); // fake click on own color
        i = choice[piece];
        if(i <= 0) return; // invalid choice; ignore
        var promoDist = (y <= nType ? ranks - 1 - py : py);
        if(i == 2 && !promoDist) return; // choice not allowed on last rank
        if(promoDepth[piece] <= promoDist) return;
        if(i == 3) { // must take from holdings
          if(!hand[y]) return; // but not there
          hand[y]--;
        }
      } else if(piece == 1 || royal[piece]) return; // by default no Pawn or King
      if(y > nType) y += 1024 - nType;
      Move(xx, yy, px, py, ex, ey, y);
      Display();
//      ShowMessage("Next move:");
      for(i=1; i<=2*nType; i++) Highlight(100, i, 0);
      if(useAI[active]) setTimeout('AiMove()', 100);
      xx = -1; px = -1; ex = -1;
      return;
    }
    if(hand[y] == 0) return;
    if(xx >= 0) xx = -1;
  }
  var same = (xx == x && yy == y);
  if(same && ex < 0 && xx < 100 && board[y][x] & 1<<29 && useAI[active]) { setTimeout('AiMove()', 100); xx = -1; return; } // null move
  if(xx < 0 || piece & 2048-1 && !same && xx < 100 && ((piece ^ board[yy][xx]) & 1024) == 0 && (piece & 511) <= nType && !sequel && !(piece & 1<<30)) {
    if(piece == 0 || (piece & 511) > nType && !pacoMode) return;
    if(xx >= 0 && !Move(xx, yy, x, y, ex, ey, 1022)) return; // opportunity to merge with own piece
    if(piece & 2048) { // blue square on own piece = relay
      ex = x; ey = y; phase *= 2;
      Display();
      GenMoves(xx, yy, 2);
      ShowMessage("<b>Move activated piece</b>");
      xx = x; yy = y; ex = ey = -1; // new mover, fake first click on it
      return;
    }
    Lowlight(); if(xx >= 0) Display();
    xx = x; yy = y; ex = -1; phase = 1; preClick = piece;
    Highlight(x, y, "#80FF80"); hx1 = x; hy1 = y;
    if(x < 100) {
      var h = board[y][x];
      var tmp = (activePlayer & 1024 ? piece >> 3 & 7 : piece & 7 | 1024);
      if(y > 1 && y < 6) tmp |= 512;
      if((piece & 511) > nType) board[y][x] = tmp, pacify = 1;
      if(!autoGate || y && y != ranks-1) GenMoves(x, y, 1);
      board[y][x] = h; pacify = 0;
    }
    ShowMessage("Click to-square:");
    RightClick(x, y);
  } else if(piece & 2048) { // blue square
    ex = x; ey = y; phase *= 2;
    Display(); // erase old markers
    GenMoves(xx, yy, 2);
    ShowMessage("<b>Enter second leg of move</b>");
  } else {
    if(xx != x || yy != y || ex >= 0) {
      if(sequel && (piece & 511) <= nType && (piece & 511) != 0 && ((sequel ^ piece) & 1024) == 0) return;
      if(!sequel && (piece & 511) && xx < 100 && (board[yy][xx] & 511) > nType) return; // combi to occupied square
      if(x < 100 && !(board[y][x] & (1<<30))) { // non-highlighted destination
        if(enforceRules) return;
        if(board[y][x] & 511) { xx = -1; Lowlight(); Display(); Click(x, y); return; } // retry as first click
      }
      if(xx < 100 && Promotion(xx, yy, x, y)) {
        m = (board[yy][xx] & 1024 ? nType : 0);
        if(promoOffset) {
          ShowMessage('<b>Promote? click here:</b> ' +
            '<span onclick="ShogiProm(1)" style="color:blue"><u>yes, please?</u></span> / ' +
            '<span onclick="ShogiProm(0)" style="color:blue"><u>no, thank you!</u></span>'); 
          px = x; py = y;
          return;
        }
        var legend = document.getElementById('legend' + active);
        if(legend) legend.style.display = 'inherit';
        if((legend == null || legend.style.display != 'none') && document.getElementById('pieceTable' + active) != null) { // can choose from table
          ShowMessage('<b>Click on icon of piece of choice</b>', 'red');
          for(i=1; i<=nType; i++) {
            px = choice[i];
            if(px > 0) {
              if(px == 3 && hand[i + m] <= 0) px = 0;
              if(px == 2 && (y == autoGate || y == ranks - 1 - autoGate)) px = 0;
              if(promoDepth[i] <= (m ? y : ranks - 1 - y)) px = 0;
            }
            if(px > 0) Highlight(100, i + m, "#8080FF");
          }
          px = x; py = y;
          return;
        } else {
          promoType = nType-1;
          if(promoChoice.length > 0 && promoChoice[0] != '*') {
             for(var i=nType; i>1; i--) if(ids[i] ==  promoChoice[0]) break;
             if(i > 1) promoType = i;
          }
        }
      }
      if(piece & 4096) { // e.p.
        m = recordSize*(gameLen-1);
        ex = game[m + 2]; ey = game[m + 3]; // set locust square on last mover
      }
      if(piece & (1<<13)) { // double push
        ex = 100; // used as flag
      }
      if(ex >= 0 && ex < 100 && board[ey][ex] & 1<<16) xx = ex, yy = ey, ex = -1; // relay
      if(ex < 0 && piece & 1<<17) ex = xx, ey = yy; // one-legger requests unload of victim
      if(piece & (1<<14)+(1<<18)) { // castling
        m = (x > xx ? 1 : -1);
        ey = yy; ex = xx; // locust square on nearest piece
        while((board[yy][ex+=m] & 511) == 0);
        if(ex == x && piece & 1<<14) x = xx + m; // adjacent castling
//statuss.innerHTML += 'castling ('+ex+','+ey+') to ('+x+','+y+')<br>';
      }
      if(Move(xx, yy, x, y, ex, ey, promoType)) return; // abort if WeirdPromotion says so
      moved++;
      if(sequel) {
        var tmp = board[y][x];
        board[y][x] = sequel;
        GenMoves(x, y, 1);
        board[y][x] = tmp;
        xx = y; yy = y; ex = -1;
        ShowMessage("Place released " + names[sequel & 511] + ':');
        var p = document.getElementById('relay');
        if(p) p.innerHTML = 'Place:<br><img src="' + MakeImage(sequel & 511, sequel & 1024 ? blackPrefix : whitePrefix) + '">';
        return;
      }
    } else {
      Lowlight(); Display();
    }
    ShowMessage("Next move:");
    if(useAI[active] && moved) setTimeout('AiMove()', 100);
    xx = -1; px = -1; ex = -1;
  }
}

function RightClick(x, y) {
  var pd = document.getElementById('piecedesc' + active);
  if(pd == null) return;
  if(x >= 100) return;
  var piece = board[y][x] & 2048 - 1 - 512;
  if(piece == 0) return;
  pd.innerHTML =
    '<tr><th>ID</th><th>white</th><th>nr</th><th>black</th><th>nr</th><th>name</th><th id="mp">move</th></tr>'
    + TableRow(piece, 1)
    + '<tr><td colspan="7" width="' + files*sqrSize + '">Click on its name to see moves on empty board.</td></tr>';
}

var downX;
var downY;
var noMouse = 0;
var down = 0;

function Down(bnr, x, y, ev) {
  if(noMouse) return;
  SwitchDiag(bnr);
  if(ev.button == 2) {
    ev.preventDefault();
    RightClick(x, y);
    return;
  }
  downX = x; downY = y; down = 1;
  curCell = "";
  Click(x, y);
}

function Up(bnr, x, y) { // catches up-click on empty square
  if(noMouse) return;
  SwitchDiag(bnr);
  if(x == downX && y == downY) return; // ignore up on static click
  Click(x, y); // on other square up counts as new click event
}

function Touch(bnr, x, y){
  noMouse = 1;
  SwitchDiag(bnr);
  if(touched > 0) { Interpose(bnr + 'y' + y + 'y' + x); return; } else
  if(down && x == downX && y == downY) { Display(); down = 0; return; }
  downX = x; downY = y; down = 1;
  curCell = "";
  Click(x, y);
}

function Copy(src, dest, l) {
  for(k=0; k<l; k++) {
    xStep[dest] = xStep[src]; yStep[dest] = yStep[src]; flags[dest] = flags[src];
    skip[dest] = skip[src]; vector[dest] = vector[src]; ranges[dest++] = ranges[src++];
  }
}

// Game navigation section

var start = new Array();
var startHand = new Array();
var game = new Array();
var comments = new Array();
var scores = new Array();
var gameLen = 0;
var displayed = 0;
var recordSize = 7;

function AddMove(x1, y1, x2, y2, epx, epy, promo) {
  p = recordSize*gameLen;
  game[p] = x1;
  game[p+1] = y1;
  game[p+2] = x2;
  game[p+3] = y2;
  game[p+4] = epx;
  game[p+5] = epy;
  game[p+6] = promo;
  displayed = ++gameLen;
}

function DisplayScore(n) {
    if(n < 1) return;
    var s = document.getElementById(n & 1 ^ flip ? "bottomscore" : "topscore");
    if(s != null) s.innerHTML = scores[n-1];
}

function StartPos () { // restore start position
  for(i=0; i<ranks; i++) for(j=0; j<files; j++) board[i][j] = start[i][j];
  for(i=0; i<=2*nType; i++) hand[i] = startHand[i];
  flagX = -1; imitatedType = 0;
}

var seeking;

function Seek(target) {
  if(target == 10003 && displayed < gameLen - 1) seeking = 1, timer = setTimeout('Seek(10003);', 1000); else seeking = 0;
  if(target >= 10002) target = displayed + 1; else
  if(target == 10001) target = displayed - 1;
  if(target > gameLen) target = gameLen;
  if(target < 0) target = 0;
  if(target == displayed) return;
ShowMessage('gameLen = ' + gameLen + ' target = ' + target + ' displayed = ' + displayed);
  if(target < displayed) {
    Lowlight(); StartPos(); Shuffle(1); displayed = 0;
  }
  while(displayed < target) {
    p = recordSize*displayed;
    if(typeof(WeirdChain) == 'function') // For determining sequel
      WeirdChain(game[p], game[p+1], game[p+2], game[p+3], 0, game[p+4], game[p+5]);
    sanMoves[displayed] = ToSAN(game[p], game[p+1], game[p+2], game[p+3], game[p+4], game[p+5], game[p+6]);
    MakeMove(game[p], game[p+1], game[p+2], game[p+3], game[p+4], game[p+5], game[p+6]);
    displayed++;
  }
  hx1 = game[p]; hy1 = game[p+1]; hx2 = game[p+2]; hy2 = game[p+3];
  Display();
  if(comments[displayed-1] != '') ShowMessage(displayed + ': ' + comments[displayed-1]); else
  ShowMessage(displayed == gameLen ? "Enter move:" : 'Position after move ' + displayed);
  if(clocks) Display(displayed), DisplayScore(displayed-1);
  var p = document.getElementById("game"); if(p) p.innerHTML = FlushSAN();
}

// Client section

var request;
request = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");
var fetchRequest = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");
var pvRequest = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");
var date = new Date();

fetchRequest.onreadystatechange = function() { if(this.readyState == 4 && this.status == 200) FetchCallback(); };
pvRequest.onreadystatechange = function() { if(this.readyState == 4) { if(this.status == 200) NextPV(); else pvCount = 0; } };

function get_url(url)
{
    request.open("GET", url, false); // synchronous request
    request.send('');
    return request.responseText;
}

function a_get_url(channel, url)
{
    channel.open("GET", url, true); // asynchronous request
    channel.send('');
}

function select_opponent(alias)
{
    document.myform.line.value = alias;
}

var listToggle = 0;

function DownloadData(pw, cmd, nr) {
  var url = server + '?game=' + gameName;
  if(!cmd) cmd = document.myform.line.value;
  url += '&cmd='  + encodeURIComponent(cmd);
  url += '&user=' + encodeURIComponent(document.myform.alias.value);
  if(pw)     url += '&pw=' + encodeURIComponent(document.myform.pw.value);
  if(pw > 1) url += '&color=' + (pw - 2); // for creating game
  if(nr > 0) url += '&nr=' + nr;          // for fetching game
  listToggle = 0;
  return get_url(url + '&t=' + date.getTime());
}

function ServerRequest(pw, cmd, nr, show) {
  newText = DownloadData(pw, cmd, nr); // no nr
  if(newText != '') {
    document.getElementById('chatbox').innerHTML = newText;
    if(show) ShowMessage(newText); // also show above board
  }
  if(show == 2) document.myform.line.value = '';
  return newText;
}

function ListGames() {
  var url = server + '?game=' + gameName;
  user = document.myform.line.value; document.myform.line.value = '';
  if(user == '' && !listToggle) user = document.myform.alias.value;
  url += '&cmd=list&user=' + encodeURIComponent(user);
  newText = get_url(url + '&t=' + date.getTime());
  document.getElementById('chatbox').innerHTML = (newText == '' ? 'No games found' : newText);
  listToggle = !listToggle;
}

var gameNr = 0;
var players = "";
var serverLen = 0;
var ticks = 0;
var elapsed = 0;
var tickInterval = 5;
var activeClock = -1;
var clockTimes = new Array();
var gameTimes = new Array();

function DisplayClock(n) {
  var t = clockTimes[n];
  var prefix = ''; if(t < 0) { t = -t; prefix = '-'; }
  var cname = (n ^ flip ? "topclock" : "bottomclock");
  var sec = t%60;
  if(sec < 10) sec = '0' + sec;
  document.getElementById(cname).innerHTML = prefix + Math.floor(t/60) + ':' + sec;
}

function Tick() {
  if(clocks) {
    --clockTimes[activeClock];
    DisplayClock(activeClock);
  }
}

function ProbeServer() {
}

var refreshCnt = 0;
var pvTarget, pvCount = 0;

function DisplayPV() {
  if(pvCount) return; // still busy
  pvTarget = document.getElementById('white');
  if(pvTarget != null) pvCount = 2, a_get_url(pvRequest, 'white.txt?t=' + date.getTime());
}

function NextPV() {
  pvTarget.innerHTML = pvRequest.responseText;
  if(--pvCount == 0) return;
  pvTarget = document.getElementById('black');
  if(pvTarget != null) a_get_url(pvRequest, 'black.txt?t=' + date.getTime()); else pvCount = 0;
}

function ClockTick() {
  if(chatID && --refreshCnt < 0) UpdateChat(), refreshCnt = 10;
  setTimeout('ClockTick();', 1000);
  if(activeClock < 0) return;
  Tick();
  if(--ticks < 0) {
    if(elapsed++ > 50) { elapsed = 0; tickInterval *= 2; } // gradually reduce probing frequency
    ticks = tickInterval;
    var tmp = players.split('-');
    if(tmp.length == 2 && tmp[activeClock] == document.myform.alias.value) return;
    if(!seeking) FetchGame(gameNr);
    DisplayPV();
  }
}

function Fid(n) {
  return String.fromCharCode(parseInt(n) + 97);
}

function Rid(n) {
  return parseInt(n) + rank1;
}

function ToSAN(ff, fr, tf, tr, ex, ey, prom) {
  var fileDis = 0, rankDis = 0, anyDis = 0, EP = '', dash = '';
  var f, r, piece;
  if(ff == 100) {
    if(fr > nType) fr -= nType;
    return ids[fr] + '@' + Fid(tf) + Rid(tr);
  }
  piece = board[fr][ff] & 1024+512-1;
  sanX = tf; sanY = tr; san = 2;
  GenPseudoMoves(ff, fr, 1);
  var sanStart = san ^ 1;
  for(r=0; r<ranks; r++) for(f=0; f<files; f++) {
    var other = board[r][f] & 1024+512-1;
    if(!other) continue;
    if(!((piece ^ other) & 1024) && ids[piece & 512-1] == ids[other & 512-1] && (f != ff || r != fr)) {
      san = sanStart;
      GenPseudoMoves(f, r, 1);
      if(san & 1) { // other piece of same type can move to to-square!
        if(ff == f) rankDis++; else if(fr == r) fileDis++; else anyDis++;
      }
    }
  }
  piece &= 512-1; san = 0;
  var ID = ids[piece], victim = board[tr][tf] & 512-1;
  if(piece == 1 && ID == 'P') { ID = ''; if(victim) fileDis++; }
  if(ex >= 0 && ey >= 0 && ex < 100) {
    dash = '-';
    if(ID == '') victim = ++fileDis; else EP = 'x' + Fid(ex) + Rid(ey);
    if(((board[ey][ex] ^ board[fr][ff]) & 1024) == 0) return (ex > ff/2 ? 'O-O' : 'O-O-O');
  }
  if(anyDis && !rankDis) fileDis++;
  prom &= 512-1; if(prom > nType) prom = 0;
  return ID + (fileDis ? Fid(ff) : '') + (rankDis ? Rid(fr) : '') + EP + (victim ? 'x' : dash) + Fid(tf) + Rid(tr) + (prom ? '=' + ids[prom] : '');
}

function FlushSAN() {
  var t = '';
  for(i=0; i<gameLen && typeof(sanMoves[i]) != 'undefined'; i++) t = t + (!(i & 1) ? (i/2 + 1) + '. ' : '') + sanMoves[i] + ' ';
  return t;
}

function FetchGame(n) {
  if(n != gameNr) {
    Seek(0); gameLen = 0; // delete old game
    elapsed = 0; tickInterval = 1; ticks = 1;
  }
  gameNr = n; oldLen = gameLen;
  var url = server + '?game=' + gameName + '&cmd=game&user=&nr=' + n;
  a_get_url(fetchRequest, url + '&t=' + date.getTime()); // asynchronous download, calls FetchCallback on completion
}

function FetchCallback() {
  newText = fetchRequest.responseText; // no password, but nr
  moveList = newText.split('\n');
  m = 0; gameLen = gameTimes[0] = gameTimes[1] = 0;
  players = moveList[0]; document.getElementById('players').innerHTML = players;
  for(i=1; i<moveList.length; i++) {
    activeClock = (i & 1); // opponent of mover
    var coords = moveList[i].split(',');
    if(coords[0] == 102) { // game ended
      comments[gameLen-1] = comments[gameLen-1] + ': <b>' + coords[1] + '</b>';
      activeClock = -1; // stop clock
      break;
    }
    game[m]   = parseInt(coords[0]); game[m+1] = parseInt(coords[1]); // convert the move to our internal format
    game[m+2] = parseInt(coords[2]); game[m+3] = parseInt(coords[3]);
    game[m+4] = -1; game[m+5] = -1; game[m+6] = 0; comments[gameLen] = ''; scores[gameLen] = ''; newTime = 0;
    for(j=4; j<coords.length; j+=4) {
      if(coords[j] > 100) {
        if(coords[j] == 103) scores[gameLen] = coords[j+1]; else
        if(coords[j] == 104 && clocks) gameTimes[activeClock] = parseInt(coords[j+1]); // sync opponent's clock (that is started)
        else comments[gameLen] = coords[j+1];
        j -= 2; continue; // comments take only two items
      }
      if(parseInt(coords[j]) == game[m+2] && parseInt(coords[j+1]) == game[m+3]) { //continuation from to-sqr
        game[m+4] = game[m+2];   game[m+5] = game[m+3];   // visited square is e.p.
        game[m+2] = parseInt(coords[j+2]); game[m+3] = parseInt(coords[j+3]); // and next true to-square
      } else if(coords[j] == 100 && coords[j+2] == game[m+2] && coords[j+3] == game[m+3]) { // drop on to-sqr
        game[m+6] = parseInt(coords[j+1]); // promotion piece
      } else { // second piece moved independently
        m += recordSize; gameLen++; // consider it a new move
        game[m]   = parseInt(coords[j]);   game[m+1] = parseInt(coords[j+1]);
        game[m+2] = parseInt(coords[j+2]); game[m+3] = parseInt(coords[j+3]);
        game[m+4] = -1; game[m+5] = -1; game[m+6] = 0;
      }
    }
    m += recordSize; gameLen++;
  }
  serverLen = gameLen; // remember length of game on server
  if(gameLen > oldLen) {
    if(displayed == oldLen) Seek(10003); elapsed = 0; ticks = tickInterval = 5;
    if(clocks) clockTimes[0] = gameTimes[0], clockTimes[1] = gameTimes[1], DisplayClock(0), DisplayClock(1);
  } else if(gameLen < oldLen) Seek(1000);
}

function SubmitMove() {
  t = '';
  for(i=serverLen; i<gameLen; i++) {
    if(i > serverLen) t += ',';
    m = recordSize*i;
    t += game[m] + ',' + game[m+1] + ',';
    if(game[m+4] >= 0) { // e.p. encoded as detour
      t += game[m+4] + ',' + game[m+5] + ',';
      t += game[m+4] + ',' + game[m+5] + ',';
    }
    t += game[m+2] + ',' + game[m+3];
    if(game[m+6]) { // promo suffix
      t += ',100,' + game[m+6];
    }
  }
  if(document.myform.line.value.length) { // append comment
    t += ',101,' + document.myform.line.value;
  }
  t = ServerRequest(1,t,gameNr,0);
  if(t != "Move accepted\n") {
    gameLen = serverLen; // clip off moves that where refused
    Seek(gameLen);
  } else  serverLen = gameLen; // or make us aware server now has them
  ShowMessage(t);
}

// Betza compilation section

var count = new Array();

function Expand(from, to) {
  var vx, vy;
  len = to - from;
  Copy(from, 0, len); // stash raw atom in save place
  for(k=0; k<=len; k++) { 
    count[k] = 0; vector[k] |= 256; // make sure following loop terminates
    while((vector[k] & (1 << count[k])) == 0) count[k]++; // skip invalid angles
  }
  while(1) {
    for(k=0; k<len; k++) { // find next valid angle combination
      while((vector[k] & (1 << count[k])) == 0) count[k]++; // skip invalid angles
      if(count[k] < 8) break; // no overflow, so done
      count[k] = 0; count[k+1]++; // wrap and add carry
      while((vector[k] & (1 << count[k])) == 0) count[k]++; // skip invalid angles
    }
    if(k == len) break; // no new valid angle combination
    Copy(0, from, len); // append new path
    dir = 0; mSlide = -1;
    for(k=0; k<len; k++) { // redirect all steps
      dir = (dir + count[k]) & 7; // according to cumulative orientation
      n = from + k;
      vx = xStep[n];
      vy = yStep[n];
      if(dir&1) {
        if(vx == 0) vx = vy; // orthogonal 8-fold: slip in diagonals
        else if(vx == vy) vy = 0;
        else  { h = vx; vx = vy; vy = h; }
      }
      if(dir&2) { h = vx; vx = vy; vy = -h; }
      if(dir&4) { vx = -vx; vy = -vy; }
      xStep[n] = vx;
      yStep[n] = vy;
      vector[n] = vx + 2*files*vy;
      if(k < len-1 && flags[k] & 1 && ranges[k] != 1) mSlide = k;
    }
    if(mSlide >= 0) { // sliding non-final leg with m rights
      if(flags[mSlide] & 0x1E) { // also has terminal rights
        Copy(from, from+len, len); // split
        flags[from+mSlide] &= ~1; // first gets the non-m rights
        from += len;
        flags[from+mSlide] &= ~0x1E; // second just m rights
      }
      for(k=from; k<=from+mSlide; k++) skip[k]++; // slip in loop directive
      Copy(from+mSlide, from+len, 1);
      skip[from+len] = 0; flags[from+len] = -1; ranges[from+len] = skip[from+mSlide] - 1;
      from++;
    }
    from += len;
    count[0]++; // step off this angle combination to trigger finding next
  }
  return from;
}

function Curve(old, vx, vy, dirSet, range, mode, angle) {
  var i, j, s;
  movePtr = old;
  if(range < 2) range = (files > ranks ? files : ranks) - 1;
  if(range > 7 && angle & 0x82) range = 7;
  s = movePtr;
  for(j=0; j<range; j++) {
    xStep[s] = vx;
    yStep[s] = vy;
    vector[s] = (j ? angle : dirSet);
    ranges[s] = 1;
    flags[s] = mode & 64+3 | (1<<20);
    skip[s] = range - j - 1;
    if(angle & 0x44) angle ^= 0x44;
    s++;
  }
  flags[s-1] = mode & 64+3 | 1024;
  if(mode & 16) flags[movePtr] |= 1 << 18;
  if(mode & 32) flags[movePtr] |= 1 << 19;
  movePtr = Expand(movePtr, s);
}

function DirSpec(c) {
  if(c == 'f') return 0xC381;
  if(c == 'b') return 0x3C18;
  if(c == 'v') return 0x9999;
  if(c == 'l') return 0xF060;
  if(c == 'r') return 0x0F06;
  if(c == 's') return 0x6666;
  if(c == 'h') return 16;
  return 0;
}

function DirSpecK(c) {
  if(c == 'f') return 0x8301;
  if(c == 'b') return 0x3810;
  if(c == 'v') return 0xBB11;
  if(c == 'l') return 0xE040;
  if(c == 'r') return 0x0E04;
  if(c == 's') return 0xEE44;
  if(c == 'h') return 16;
  return 0;
}

var movePtr = 100;

function BError(msg) {
  var p = document.getElementById('BetzaError');
  if(p) p.innerHTML = (msg == '' ? '' : '<span style="background:yellow"><b>Error:</b> ' + msg + '</span>');
}

function BetzaParse(piece, betza) {
  first[piece] = ++movePtr; // reserve one
  p = 0;
  while(p < betza.length) loop: {
    q = p; oldPtr = movePtr; sym = -1; range = 1; mode = 0; dirSet = 0;
    do { // scan forward to atom
      c = betza[q++];
      if(c == 'A') { vx = 0; vy = 2; sym = 2; } else
      if(c == 'B') { vx = 0; vy = 1; sym = 2; range = 0; } else
      if(c == 'C') { vx = 1; vy = 3; sym = 3; } else
      if(c == 'D') { vx = 0; vy = 2; sym = 0; } else
      if(c == 'F') { vx = 0; vy = 1; sym = 2; } else
      if(c == 'G') { vx = 0; vy = 3; sym = 2; } else
      if(c == 'H') { vx = 0; vy = 3; sym = 0; } else
      if(c == 'I') { vx = 0; vy = 1; sym = 0; mode = 1<<24; } else
      if(c == 'J') { vx = 2; vy = 3; sym = 3; } else
      if(c == 'K') { vx = 0; vy = 1; sym = 1; } else
      if(c == 'L') { vx = 1; vy = 3; sym = 3; } else
      if(c == 'N') { vx = 1; vy = 2; sym = 3; } else
      if(c == 'O') { vx = 0; vy = 1; sym = 0; mode = 8+(1<<23); }  else
      if(c == 'Q') { vx = 0; vy = 1; sym = 1; range = 0; } else
      if(c == 'R') { vx = 0; vy = 1; sym = 0; range = 0; } else
      if(c == 'W') { vx = 0; vy = 1; sym = 0; } else
      if(c == 'Z') { vx = 2; vy = 3; sym = 3; } else
      if(c == 'U') { vx = 0; vy = 0; sym = 0; range = -1; } else
      if(c == '@') { vx = 2; vy = 2; sym = 5; } else
      if(c == c.toUpperCase()) BError('unknown atom ' + c + ' ignored'); else
      if(q >= betza.length) { BError('no atom at end'); p = q; break loop; } // no valid atom
    } while(sym < 0);
    a = q - 1; var origRange = range;
    if(q < betza.length) {
      if(c == betza[q] && c != 'K' && c != 'Q' && c != 'R' && c != 'B') {
        q++; range = 0;
      } else while('X' == betza[q] || 'Y' == betza[q]) {
        if('X' == betza[q++]) {
          if(sym == 2) vx = vy, vy += 3, sym = 3; else vy += 3;
        } else {
          vy += 2; if(sym != 2) vx += 2, sym = 3;
        }
      }
      c = betza[q];
      if(c <= '9' && c >= '0') {
        range = parseInt(betza.substring(q)); q++; if(range > 9) q++;
      } else if(c == '*') range = -1, mode |= (1<<14), q++;
    }
    if(betza[a] == 'O' && range < 1) {
      if(range == 0) { // universal leaper
      }
      p = q; continue; // ignore null-move for now
    }
    xStep[movePtr] = vx;
    yStep[movePtr] = vy;
    ranges[movePtr] = range;
    var ii = 64; // initial flag
    for(m=p; m<a; m++) {
      c = betza[m]; d = DirSpec(c);
      if(d) { // directional modifier
        if(sym == 0) { // orthogonal never combines
          dirSet |= d & 0xFF; // use narrow
        } else if(d == 16) {  // chiral
          c = betza[m+1];
          if(sym == 3) {
            if(c == 'r') { dirSet |= 0x55; m++; } else
            if(c == 'l') { dirSet |= 0xAA; m++; }
          }
        } else { // try to combine
          if(sym != 3) { // K-system: non-degenerate octal
            d = DirSpecK(c);
            d2 = DirSpecK(betza[m+1]);
          } else d2 = DirSpec(betza[m+1]);
          if(d2) { // other directional modifier follows
            if(d2 == 16) { // h: use wide set
              dirSet |= (d >> 8); m++;
            } else if(sym == 1) { // K
              if(d & d2 & 0xFF00) { // two wide sets intersect
                if(d & (d2 >> 8)) dirSet |= d & 0xFF; // doubled direction like ff
                else dirSet |= (d & d2) >> 8; m++;    // orthogonal intersects wide sets
              } else dirSet |= d & 0xFF; // or narrow if uncombinable
            } else if(sym == 2) { // F
              if(d & d2 & 0xFF00) { // two wide sets intersect
                dirSet |= (d & d2) >> 8; m++; // use that
              } else dirSet |= d >> 8;
            } else { // sym=3, N
              d2 &= (d >> 8); // intersect wide of 1st with narrow 2nd
              if(d2) m++; else d2 = d & 0xFF; // use wide for diagonal
              dirSet |= d2;
            }
          } else dirSet |= (sym & 1 ? d & 0xFF : d >> 8);
        }
      } else
      if(c == 'm') { mode |= 1; } else   // move
      if(c == 'c') { mode |= 2; } else   // capture
      if(c == 'e') { mode |= 4; } else   // e.p. (8 = castle)
      if(c == 'p') { mode |= 16; } else  // hop
      if(c == 'g') { mode |= 48; } else  // grasshop (range toggle)
      if(c == 'y') { mode |= 33; } else  // spontaneous range toggle
      if(c == 'i') { mode |= ii; } else  // initial or iso
      if(c == 'n') { mode |= (vy > 1 ? (mode & 192 && vy < 5 ? 128|1<<14 : 128) : 1<<23); } else // lame (in or nn creates e.p. rights)
      if(c == 'j') { mode += 256; } else // jumping
      if(c == 'x') { mode |= 2048;} else // relay
      if(c == 'd') { mode |= 4096;} else // destroy
      if(c == 'k') { mode |= 1<<13;}else // check
      if(c == 'z') { mode |= 1<<18;}else // crooked
      if(c == 'q') { mode |= 1<<19;}else // circular
      if(c == 'o') { mode += 1<<16;}else // cylinder
      if(c == 'u') { mode |= 1<<21; if(skip[movePtr-1]) flags[movePtr-1] |= 1<<22; }else // unload
      if(c == 'w') BError("'w' doesn't mean anything"); else
      if(c == 'a') { // multi-leg
        // store the now-finished leg
        if(dirSet == 0) dirSet = (movePtr == oldPtr ? 0xFF : 0xEF);
        if(sym == 0) dirSet &= 0x55; else if(sym == 2) dirSet &= 0xAA; // 4-fold
        if((mode & 31+2048+4096) == 0) mode |= 1; // default in non-final leg is 'm'
        if(mode & 32) origRange = range = !origRange;    // toggle range
        else range = origRange;
        for(k=oldPtr; k<movePtr; k++) skip[k]++;
        skip[movePtr] = 1;
        vector[movePtr] = dirSet;
        flags[movePtr++] = mode;
        if(mode & 2) modes |= 4; modes |= mode & 16;
        dirSet = 0; mode = 0; // initialize for next leg
        xStep[movePtr] = vx;
        yStep[movePtr] = vy;
        ranges[movePtr] = range;
        sym = 1; // use non-deg octal symmetry for continuation legs direction specs
        ii = 1<<15; // 'i' in continuation leg stands for 'iso'
      }
    }
    flags[movePtr] = mode;
    if(dirSet == 0) dirSet = (movePtr == oldPtr ? 0xFF : 0xEF);
    if(mode & 1<<24) dirSet &= 17; // imitators v-only 
    if(movePtr == oldPtr) { if(sym == 0) dirSet &= 0x55; else if(sym == 2) dirSet &= 0xAA; }
    vector[movePtr] = dirSet;
    modes |= mode & 16+8;
    if(!(mode & 3<<18)) {
    if(mode & 16+8) { // old-fashioned p or g in final leg, or O atom
      skip[movePtr] = 0; // setup to skip next leg
      flags[movePtr++] = mode & 48+64+8; // split step into hop
      xStep[movePtr] = vx; // and final leg in same direction
      yStep[movePtr] = vy;
      vector[movePtr] = 0x01; // forward
      for(k=oldPtr; k<movePtr; k++) skip[k]++;
      if(mode & 8) { // castling
        ranges[movePtr-1] = 0;  // first leg slides to Rook
        if(mode & 256) flags[movePtr-1] |= 1<<25; // remember j
        yStep[movePtr] = range; // second leg will start at from-sqr
        mode = 8+1; ranges[movePtr] = 1; // and does not capture or slide
      } else {
        ranges[movePtr] = (mode & 32 ? !range : range > 1 ? -2 : range);
        mode &= 3;
      }
    } else
    if(mode & 256 && vy == 1) { // j on W or F: ski-slide
      skip[movePtr] = 1; // setup to skip next leg
      ranges[movePtr] = 1;
      flags[movePtr++] = mode & 64 | 17; // split step into hop
      xStep[movePtr] = vx; // and final leg in same direction
      yStep[movePtr] = vy;
      vector[movePtr] = 0x01; // forward
      ranges[movePtr] = range - 1;
      mode &= ~(64 + 256);
    }
    }
    skip[movePtr] = 0;
    asym[piece] = ((0x101*vector[oldPtr] >> 4 & 0xFF) != vector[oldPtr]);
    if((mode & 7+4096+2048+(1<<13)) == 0) mode |= 3;  // default final leg is 'mc'
    flags[movePtr++] = mode | 1024; // termination flag
    j = 1;
    if(mode & (1<<18)) Curve(oldPtr, vx, vy, dirSet, range, mode, 0x40), Curve(movePtr, vx, vy, dirSet, range, mode, 4); else
    if(mode & (1<<19)) Curve(oldPtr, vx, vy, dirSet, range, mode, 0x80), Curve(movePtr, vx, vy, dirSet, range, mode, 2); else
    movePtr = Expand(oldPtr, movePtr); // separate all orientations and shapes of the paths described by this atom
    if((mode & 64+3) == 64+3) modes |= 1; if(mode & 4096) modes |= 2;
    p = q; // next atom
  }
  flags[movePtr++] = 0; // sentinel
}

function AddEvents(i, mute) {
  if(mute) return '';
  return ' id="' + active + 'y' + i + 'y100" ondragend="Relay()" onmousedown="Down('
                 + active + ',100,' + i + ',event)" ontouch="Touch(' + active + ',100,' + i + ')"';
}

function AddCount(i, c, w, mute) {
  if(noCounts) return '';
  if(mute) return '<td></td>';
  return '<td id="' + active + c + i + '" width="30">' + w + '</td>';
}

function MakeImage(n, prefix) {
  if(n > nType) return graphDir + 'White' + ids[n >> 3 & 7] + ids[n & 7] + '.png';
  var im = imag[n];
  if(im.indexOf('%') < 0) return graphDir + prefix + im; // from standard set
  var p = im.split('%'); // custom graphics; replace % by color prefix
  return p[0] + prefix + p[1];
}

function TableRow(n, mute) {
    var i = n & 511;
    var j = i + nType;
    var w = hand[i]; if(w == 0) w = "-";
    var b = hand[j]; if(b == 0) b = "-";
    var row  = '<td width="20"><b>' + ids[i] + '</b></td>';
    row += '<td' + AddEvents(i, mute) + ' width="' + sqrSize + '"><img src="' + MakeImage(i, whitePrefix) + '"></td>' + AddCount(i, 'w', w, mute);
    row += '<td' + AddEvents(j, mute) + ' width="' + sqrSize + '"><img src="' + MakeImage(i, blackPrefix) + '"></td>' + AddCount(i, 'b', b, mute);
    row += '<td onmousedown="ShowMoves2(' + active + ',' + n + ',-2,0)">' + names[i] + '</td>';
    row += '<td id="m' + active + 'x' + (mute ? 0 : i) + '" onclick="DefinePiece(' + (mute ? 0 : i) + ')">' + moves[i] + '</td>';
    return '<tr>' + row + '</tr>';
}

var pieceHeader = '<tr><th>ID</th><th>white</th><th>nr</th><th>black</th><th>nr</th><th>name</th><th id="moval', ph2 = '" onclick="DisplayMV(', ph3 = ');">move</th></tr>';

function BetzaCompile() {
  // build dictionary of pieces (also to be used as holdings)
  var tab = '<tr><td style="background:#E0E0FF" colspan="'+ (noCounts ? 5 : 7) + '" alignn="center">Click on piece name to see its move diagram.';
  if(diagList[active].innerHTML.search('printversion') > 0)
  tab += ' (<span onclick="ShowAll(' + active + ')" style="color:blue;"><u>print version</u></span>)</td></tr>';
  tab += pieceHeader;
  var tab2 = '<li style="background:#8080FF;">Click below to display piece moves:</li>';
  if(noCounts) { tab = tab.split('<th>nr</th>'); if(tab.length == 3) tab = tab[0] + tab[1] + tab[2]; } tab += active + ph2 + active + ph3;
  movePtr = 100; oldImag = graphDir + whitePrefix + blackPrefix; modes = 0;
  flags[first[0]=movePtr++] = 0; // empty square has no moves
  for(var i=1; i<=nType; i++) {
    tab += TableRow(i, 0);
    BetzaParse(i, moves[i]);
    imitator[i] = (moves[i].split('I').length > 1);
    var sqrs = (startSqr[i].length && startSqr[i].length < 10 ? ' (' + startSqr[i] + ')' : '');
    if(!promoOffset || i <= promoOffset)
      tab2 += '<li onclick="ShowMoves2(' + active + ',' + i + ',-2,0)">' + (names[i].toUpperCase())[0] + names[i].substring(1) + sqrs + '</li>';
  }
  var p = document.getElementById("pieceTable" + active);
  if(p != null) p.innerHTML = tab;
  p = document.getElementById(satellite + "List");
  if(p != null) p.innerHTML = tab2;
  p = document.getElementById(satellite + "Colors");
  if(p != null) p.innerHTML = ColorLegend();
}

var checkFlag = 0, testFlag = 0, blockFlag = 0;

function GenMoves(x, y, legMask) {
  var king = board[y][x];
  if(royal[king & 512-1]) {
    var r, f, opponent = king & 1024 ^ 1024;
    board[y][x] = 0; checkFlag = 1<<15;
    for(r=0; r<ranks; r++) for(f=0; f<files; f++) {
      var p = board[r][f];
      if(p & 512-1 && (p & 1024) == opponent) GenPseudoMoves(f, r, 1);
    }
    board[y][x] = king; testFlag = checkFlag; checkFlag = 0;
  }
  GenPseudoMoves(x, y, legMask);
  testFlag = 0;
}

function GenPseudoMoves(x, y, legMask) {
  piece = board[y][x];
  d = first[piece & 512-1];
  if(piece & 1024) s = -1; else s = 1;
  GenInner(x, y, legMask, piece, d, s, ~0);
}

function GenInner(x, y, legMask, piece, d, s, mask) {
  var len, l, vx, vy, sx, sy, f;
  for(f = flags[d]; f; f = flags[++d]) {
    var hop = 0, ff;
    blockFlag = 0;
    if(f < 0) { // loop directive
      sx = sx1 + s*xStep[d]; sy = sy1 + s*yStep[d]; leg = leg1; // continue suspended slide
      if(sx < 0 || sx >= files || sy < 0 || sy >= ranks || (board[sy][sx] & 2047)) { d += skip[d]; continue; }
      sx1 = sx; sy1 = sy; ++len; l = 0; // remember again
      d -= ranges[d]; f = flags[d]; // redo follow-up leg
      if(f & 1<<21) {
        if(leg & legMask) epx = sx, epy = sy;
        if(leg == 1 && ex >= 0 && (sx != ex || sy != ey)) leg = 0;
        leg += leg;
      }
    } else {
      f |= (f & ~mask & 2) << 12; f &= mask;
      if((f & 64) && (piece & 512)) { d += skip[d]; continue; } // not virgin
      sx = x; sy = y; len = l = 0; ff = f; hop = f >> 18 & 3;
      if(f & 1<<24) { GenInner(x, y, legMask, piece, first[imitatedType&511], s*yStep[d], f | ~3); continue; }
      if(!xStep[d] && !yStep[d]) {
        for(i=0; i<files; i++) for(j=0; j<ranks; j++) {
          victim = board[j][i] & 2048-1;
	  foe = (victim ^ piece) & 1024;
          if((i != x || j != y) && f & 2) board[j][i] |= checkFlag;
          if(!victim) { if(f & 1) Highlight(i, j, "#FFC000"); } else
          if(foe) { if(f & 2) Highlight(i, j, "#FF0000"); } else
          if(f & 4096) Highlight(i, j, "#0000FF");
        }
        continue;
      }
      if(f & 8 && legMask & 16) Highlight(x+xStep[d+1], y+yStep[d+1], "#00A000");
      leg = 1; epx = -1;
    }
    vx = s*xStep[d]; vy = s*yStep[d];
    if(f & 8 && s < 0 && castleFlip) vx = -vx; hx = (vx*3 + 4) >> 3; hy = (vy*3 + 4) >> 3;
    r = ranges[d]; var successor = 0;
    if(r == -1) { r = (piece & 1024 ? y - (ranks - 1 >> 1) : (ranks >> 1) - y) - 1; if(r <= 1) r = 1, f &= ~(1<<14); }
    do {
      sx += vx; sy += vy; l++;
      if(ff & (3<<16)) { if(sx == x && sy == y) break; if(sx >= files) sx -= files; if(sx < 0) sx += files; } // cylinder board
      if(ff & (1<<17)) { if(sx == x && sy == y) break; if(sy >= ranks) sy -= ranks; if(sy < 0) sy += ranks; } // toroidal board
      if(sx < 0 || sx >= files || sy < 0 || sy >= ranks || board[sy][sx] == 250
         || (f & 128) &&  (board[sy-hy][sx-hx]&511 || board[sy-vy+hy][sx-vx+hx]&511)
         || (f & 256) && !(board[sy-hy][sx-hx]&511 || board[sy-vy+hy][sx-vx+hx]&511)) { d += skip[d]; break; }
      victim = (sx == x && sy == y) ? 0 : board[sy][sx] & 2048-1; // no self-interaction
      if((victim & 511) == 250) { d += skip[d]; break; } // hole
      if(f & 1<<15) { if(l < len && !victim) continue; if(l > len || l < len && victim) { d += skip[d]; break; } }
      if(f & 2+(1<<13)) board[sy][sx] |= checkFlag; // mark attacked squares
      if(!(f & 8 && r <= 0 && -r >= xStep[d+1] && -r >= -xStep[d+1])) // not for squares between R and K-to
        blockFlag |= board[sy][sx] & testFlag; // record passing through check
      if(victim) {
        foe = (victim ^ piece) & 1024 | (victim & 511) > nType;
        if(f & 2048 && f & 1024) { if(!foe && (victim ^ piece) & 511)  GenInner(x, y, legMask, piece, first[victim & 511], s, mask & ~2048); break; }
        canCapt = (foe ? f & 2 || f & 1<<13 && royal[victim & 512-1] : f & 2048+4096);
        if(canCapt && hop <= 0) {
          if(f & 1024 + (1<<20)) {
            if(leg & legMask || sx == ex && sy == ey) Highlight(sx, sy, f & 4096 ? "#0000FF" : "#FF0000");
            if(epx >= 0) { Highlight(epx, epy, "#00FFFF"); board[epy][epx] |= 2048; }
            if(f & 1<<21) board[sy][sx] |= 1<<17;
          } else {
            if(leg & legMask) { epx = sx; epy = sy; }
            if(leg == 1 && ex >= 0 && (sx != ex || sy != ey)) leg = 0;
            if(leg && f & 2048) board[sy][sx] |= 1<<16;
            leg += leg;
          }
        }
        if(f & 8) { // castling
          if(skip[d] == 0) { Highlight(sx, sy, "#00A000"), board[sy][sx] |= (1<<18); break; } else { // 2nd leg hits own Rook
            if(victim & 512 || (sx != aSide && sx != hSide) && ((victim & 511) > nType || moves[victim & 511] != 'R')) { d += skip[d]; break }
            rx = sx; ry = sy; sx = x; sy = y; // rewind to do 2nd leg
        } } else
        if(!(f & 16)) { // cannot hop
          if(hop > 0) hop -= 4; else
          if(!canCapt || f & (1<<20)) { d += skip[d]; break; } // fail also skips subsequent legs
          if(f & 1024) break; // final leg ends path
        }
      } else if(f & 1024 || !(f & 1)) { // final leg or no m rights
        var epHit = 0;
        if(f & 4 && flagX >= 0) {
          var ty = flagY, dy = (lastY > flagY ? 1 : -1), tx = flagX, dx = (lastX - flagX)*dy/(lastY - flagY);
          while(!board[ty+=dy][tx+=dx]) if(ty == sy && tx == sx) epHit++;
        }
        if(epHit) {
          Highlight(sx, sy, "#FF0000"); board[sy][sx] |= 4096;
        } else if(f & 1<<14) {   // e.p.-rights creating lame push
          board[sy][sx] |= 8192; // mark target so e.p. rights can be set
        }
        if(f & 1) {
          if(hop > 0) break;
          var both = (!(f & 128) && !successor && sx == x + vx && sy == y + vy && vx*vx + vy*vy > 2 ? "#FFC000" : "#FFFF00");
          if(f & 8) {
            if(vx > 1 || vx < -1) rx = sx, ry = sy;
            Highlight(rx, ry, "#00A000"); board[ry][rx] |= (1<<14);
          }
          else if(leg & legMask) Highlight(sx, sy, (ff & 64 ? (f & 2 ? "#C0C000" : "#00C000") : (f & 2 ? both : "#00FF00"))), board[sy][sx] |= 1<<29;
          if(epx >= 0) { Highlight(epx, epy, "#00FFFF"); board[epy][epx] |= 2048; }
        } else if(legMask & 16) {
          if(f & 1024) Highlight(sx, sy, f & 4096 ? "#0000FF" : "#FF0000");
          else if(f & 16+2 && ex < 0) Highlight(sx, sy, f & 2 ? "#00FFFF" : "#C0C0C0"), spare[sy][sx] |= (1 | f & 2) << 28;
          else if((f & 17) == 1 && ex < 0) /*Highlight(sx, sy, "#E0E0E0"),*/ spare[sy][sx] |= 1 << 28;
        }
        if(--r != 0) continue; // continue ray scan if range permits
        d += skip[d]; break;   // fail if it doesn't
      } else if(f & 1<<20) {
        if(f & 1 && hop <= 0) Highlight(sx, sy, "#FFFF00");
        if(hop == -1) { d += skip[d]; break; }
      } else {
        if(r <= 0) sx1 = sx, sy1 = sy, len = l, leg1 = leg; // for non-final slider remember where we are
        if(legMask & 16 && f & 2 && (f & 17) != 17 && ex < 0) Highlight(sx, sy, f & 2 ? "#00FFFF" : "#C0C0C0"), spare[sy][sx] |= (1 | f & 2) << 28;
        else if(legMask & 16 && (f & 17) == 1 && ex < 0) /*Highlight(sx, sy, "#E0E0E0"),*/ spare[sy][sx] |= 1 << 28;
      }
      if(f & 1<<22) { // pre-unload
        if(leg & legMask) { epx = sx; epy = sy; }
        if(leg == 1 && ex >= 0 && (sx != ex || sy != ey)) leg = 0;
        leg += leg;
      }
      r = ranges[++d]; successor = 1; if(r == -2) { r = ranges[d-1] - l; if(r == 0) break; }
      f = flags[d]; vx = s*xStep[d]; vy = s*yStep[d]; len = l; victim = l = 0; if(!(f&8)) blockFlag = 0; // next leg
      if(f & 8 && s < 0 && castleFlip) vx = -vx; hx = (vx*3 + 4) >> 3; hy = (vy*3 + 4) >> 3;
    } while(!victim);
  }
}

var msp = 0;
var moveStack = new Array();
var moveType  = new Array();

var kills, eps; // locust-square stack
var killX = new Array();
var killY = new Array();
var hop, s, stm, imi, imiMask, promoTest, ep, throughCheck, khit, kind;
var bMap = new Array();

var Emit;

function StackMove(ff, fr, tf, tr, unload) {
  var m = [ff, fr, tf, tr]; // basic move
  var j, extra = 0, type = board[fr][ff] & 511;

  if(zonal && BadZone(tf, tr, type, board[fr][ff] & 1024)) return;       // enforce custom piece confinement

  throughCheck |= khit;

  if(autoGate) {
    if(fr == 0 || tr == 0 || fr == ranks-1 || tr == ranks-1) return;     // reject moves to or from gating rank
    if(!(board[fr][ff] & 512) && (fr == 1 || fr == ranks-2)) {           // virgin piece moves from back-rank
      var gr = (fr == 1 ? 0 : ranks-1), gf = ff;                         // calculate gate
      if(kills > 1 && (board[gr][ff] & 511) == 250) gf = killX[kills-1]; // castling, try also at Rook
      if((board[gr][gf] & 511) != 250)
        unload = 0, AddKill(gf, fr), AddKill(gf, gr), extra = 2;         // add stepping gated piece on-board
  } }

  j = kills; m[-2] = kills + 2 - eps; m[-5] = unload;                    // usually 'undefined'
  for(var i=0; i<kills; i++) m[2*i+4] = killX[--j], m[2*i+5] = killY[j]; // append any locust squares
  kills -= extra;

  if(type <= maxPromote) {  // promotion mess (but only for promotable types)

    var enter = (stm ? tr < promoZone : tr >= ranks - promoZone);        // ends in zone
    var leave = (stm ? fr < promoZone : fr >= ranks - promoZone);        // starts in zone

    if(promoOffset) { // Shogi style: fixed type or defer, but tricky as to when you can do it

      if(holdingsType ? enter || leave                                   // with drops all zone touches
                      : (leave ? board[tr][tf] & 2047 : enter)) {        // enter or capture within
        var promo, c = [];
        for(i=0; m[i] != undefined; i++) c[i] = m[i]; c[-2] = m[-2];
        type += promoOffset; if(type > nType) type = promoOffset + 1;    // non-existent defaults to Tokin
        c[-1] = (promoTest ? WeirdPromotion(ff, fr, tf, tr, type + stm, m[4], m[5]) :  type + stm);
        if(c[-1] != 1022) moveStack[msp++] = c;
      } // falls through to do deferral

    } else if(enter) { // Chess style, generate all choices

      for(var k=1; k<=nType; k++) {
        var p = choice[k];
        var col = (board[fr][ff] & 1024 ? nType : 0);
        var promoDist = (col ? tr : ranks - 1 - tr);
        if(p && (p != 2 || !promoDist)           // enforce *
             && (promoDist <= promoDepth[k])     // enforce rank suffix
             && (p != 3 || hand[k + col] > 0)) { // enforce !
          var c = [];
          for(i=0; m[i] != undefined; i++) c[i] = m[i]; c[-2] = m[-2];
          c[-1] = (promoTest ? WeirdPromotion(ff, fr, tf, tr, k + stm, m[4], m[5]) :  k + stm);
          if(c[-1] != 1022) moveType[msp] = kind + 32, moveStack[msp++] = c;
      } }
      return; // no fall through

  } }

  // non-promotion attempt
  m[-1] = (promoTest ? WeirdPromotion(ff, fr, tf, tr, 0, m[4], m[5]) : 0); // can still be custom promotion
  if(m[-1] == 1022) return;                                                // or rejection code
  moveType[msp] = kind; moveStack[msp++] = m;
}

function AddKill(x, y) { killX[kills] = x; killY[kills++] = y; }

function IsEpSqr(x, y) {
  for(var k=ep[-2]; ep[k] != undefined; k+=2) // loop over e.p. squares
    if(ep[k] == x && ep[k+1] == y) return 1;  // we are on one
  return 0;
}

function TryEP(ff, fr, x, y, f, iso, royal) {
  AddKill(ep[2], ep[3]); khit += royal;       // make last mover locust victim
  if(f & 1024+(1<<20)) Emit(ff, fr, x, y);    // and emit its capture
  else NextLeg(ff, fr, x, y, iso, d+1);       // or continue with next leg
  kills--; khit -= royal;                     // clean up to continue for non-e.p.
}

function NextLeg(ff, fr, x, y, iso, d) {
  var f = flags[d];         // get leg params
  var rg = ranges[d];
  var vx = s*xStep[d];
  var vy = s*yStep[d];
  var hx = (vx*5 + 8) >> 4; // half-step (actually 5/16) rounded
  var hy = (vy*5 + 8) >> 4; // give 1/-1 for step 2, 3 and 4
  var len = 0, loop = 0, m;
  var isolen = (f & 1<<15 ? iso : 0);
  var flip = (f & 8 && castleFlip ? s : 1); // should perhaps be done for all move types?
  vx *= flip;
  f |= (f & ~imiMask & 2) << 12;            // non-imitated c could become k
  f &= imiMask;
  if(!rg) rg = 100;
  if(f & 1<<24) { // imitator
    var olds = s; s *= yStep[d];
    imiMask = f | ~(7|3<<12);
    NewInner(ff, fr, first[imi], imiMask);
    s = olds; return;
  }
  if(!(vx|vy)) return; // TODO: Universal Leaper
  if(rg == -1) { // W*
    rg = (vy < 0 ? y - (ranks - 1 >> 1) : (ranks >> 1) - y) - 1;
    f &= ~(1<<14);
    if(rg > 1) f |= 1<<23; else rg = 1;     // generates e.p. rights if not limited to 1
  }
  if(f & 1<<21) AddKill(x, y);              // signal 'unload' as locust capture on empty

  ray: { do { // ray scan along current leg

    // next square, off-board test and wrapping
    x += vx;
    if(x < 0)      { if(f & 3<<16) x += files, loop++; else break; }
    if(x >= files) { if(f & 3<<16) x -= files, loop--; else break; }
    y += vy;
    if(y < 0)      { if(f & 2<<16) y += ranks, loop += 16; else break; }
    if(y >= ranks) { if(f & 2<<16) y -= ranks, loop -= 16; else break; }

    // test (non-)jumping
    if(f & 128+256) { // j- or n-leg
      var ix = 0, iy = 0;
      while((iy += hy) != vy) {
        ix += hx;
        var occup = board[y-iy][x-ix] & 511;
        if(f & 256 ? !occup : occup) break ray;       // jumped square(s) not as required
        if(f & 1<<14) AddKill(x - ix, y - iy), eps++; // use locust squares to create e.p. rights
      }
    }
    if(f & 8 ? len < Math.abs(xStep[d+1]) : f & 1<<23 && len) { // leave trail of e.p. rights along path
      AddKill(x - vx, y - vy); eps++;                           // as locust squares (W*, n slides)
    }

    len++; m = bMap[2048*y+x]; if(m) kind |= m;
//document.getElementById("statuss").innerHTML += 'f=' + f + ' (' + x + ',' + y + ') rg=' + rg + ' len=' + len + '<br>';
    // step was successfully made; check what we hit
    var victim = board[y][x] & 2047;            // strips marker flags
    if(x == ff && y == fr && !loop) victim = 0; // ignore self unless wrapped

    if(ep && royal[ep[-6]&511] && IsEpSqr(x, y) // virtual royal on this square
          && len >= isolen && f & 2+(1<<13))    // and our move can capture here
      TryEP(ff, fr, x, y, f, rg>1?len:iso, 1);  // emit the e.p. capture (if subsequent legs allow)

    if(victim) {

      // occupied square (so leg must end here)
      if(victim == 250) break;                          // hole is also off-board
      if(len < isolen) break;                           // too short i leg
      if(hop > 0) {                                     // curve waiting for hop
        if(f & 1024) break;                             // but no room left to land
        hop -= 4; NextLeg(ff, fr, x, y, 0, d+1); break; // register the hop and continue curve
      }

      // figure out whether we hit what we needed (c k p/g: foe, d x p/g: friend)
      var canCapt, foe = victim - stm & 1024;
      if(foe) canCapt = f & 2 || f & 1<<13 && royal[victim & 511]; // c or k can capture foe
      else {
        canCapt = f & 4096;                                        // d can capture friend
        if(f & 8) {                                                // 1st leg of castling is unlimited slide
          var jj = f >> 25 & 1;
          if(x != aSide + jj && x != hSide - jj) break;            // did not reach corner
          var rook = board[y][x] & 2047;                           // corner piece
          if(rook & 512) break;                                    // rook not virgin
          var xx = ff+xStep[d+1]*s*flip, yy = fr+yStep[d+1];       // King target from next leg
          AddKill(xx - vx, yy - vy);                               // 'unload' Rook next to it
          AddKill(x, y);                                           // and locust-capt own Rook
          Emit(ff, fr, xx, yy, rook|512);                          // add 2nd leg now
          break;                                                   // cannot have other modes
        }
        if(f & 2048) {                                             // x leg (induction) only on friends
          if(f & 1024) {                                           // final leg, borrow moves from victim
            if(board[fr][ff] - victim & 511) NewInner(ff, fr, first[victim & 511], imiMask & ~2048); // but not if same type
          } else NextLeg(x, y, x, y, rg>1?len:iso, d+1);           // induce remaining legs on victim
        }
      }
      if(canCapt) {                                 // common treatment of c d
        if(f & 1024+(1<<20)) Emit(ff, fr, x, y);    // final leg or optional curve termination: generate capture
        else {                                      // non-final leg
          AddKill(x, y);                            // add locust square for victim
          NextLeg(ff, fr, x, y, rg>1?len:iso, d+1); // and try next leg
          kills--;                                  // clean it up (we might still do hop)
        }
      }
      if(f & 16) NextLeg(ff, fr, x, y, rg>1?len:iso, d+1); // p/g leg never final; do nothing here and continue
      break; // leg always terminates at obstacle

    } else {

      // empty square (so leg might continue)
      if(ep && f & 4 && IsEpSqr(x, y))                          // we hit an e.p. square and are e.p.-capable
        TryEP(ff, fr, x, y, f, rg>1?len:iso, 0);                // emit the capture
      if(!(f & 1) || len < isolen) continue;                    // no m or too short (i!), so no move to here
      if(f & 1024+(1<<20)) {                                    // final leg or optional curve termination
        if(hop <= 0) Emit(ff, fr, x, y);                        // we have move if not waiting for hop (which only curves do)
      } else NextLeg(ff, fr, x, y, rg>1?len:iso, d+1);          // otherwise try remaining legs from here
      if(f & 1<<20 && !(f & 1024)                               // curve in a non-final leg
                   && hop != -1) NextLeg(ff, fr, x, y, 0, d+1); // must be continued if not after g hop
      if(isolen > 0) break;                                     // do not continue past equal length

    }

  } while(len < rg); } // do next square of ray, when range allows
}

function NewInner(ff, fr, d, mask) { // generate move set 'd' from square (ff,fr)
  var f;
  while((f = flags[d])) {      // for all directions
    if(!(f & 64) ||                  // no virginity required
       !(board[fr][ff] & 512)) {     // or piece is virgin
      kills = eps = khit = kind = 0; // prepare kill stack
      var b = bMap[2048*fr+ff]; if(b) kind = b;
      hop = f >> 18 & 3;             // broadcast whether z or q trajectory is p or g
      imiMask = mask;
      if(f & 64 && royal[board[fr][ff] & 511]) killX[kills] = ff, killY[kills++] = fr, eps = 1; // forbid initial move out of check
      NextLeg(ff, fr, ff, fr, 0, d); // try generating the (multi-leg?) move
    }
    d += 1 + skip[d];                // next move
  }
}

function NewGen(x, y) {
  var piece = board[y][x];                // get piece on given square
  stm = piece & 1024;                     // broadcast side to move
  s = 1 - (2 & stm >> 9);                 // set up flipping of black moves
  NewInner(x, y, first[piece & 511], ~0); // generate its moves
//document.getElementById("statuss").innerHTML = t;
}

function GenAll(side) {
  var r, f;
  Emit = StackMove;
  throughCheck = 0;
  for(r=0; r<ranks; r++) for(f=0; f<files; f++) {
    var piece = board[r][f];
    if(piece && (piece & 1024) == side) NewGen(f, r);
  }
}

var pieceValue = new Array();
var baseValue = new Array();
var center, nodes, threshold, depth = 2;
var captCodes = new Array();
var code = new Array();

function ScoreMove(m, last) {
  var victim, piece = board[m[1]][m[0]];
  var stm = piece & 1024, type = piece & 511;
  var gain, specials = 0;
  // move data:       -8    -7    -6     -5     -4      -3    -2   -1     0     1    2   3    4      5 
  //            ... kill1 victim piece unload material gain flags promo fromX fromY toX toY kill1X kill1Y ...
  // so the occupant of (2k,2k+1) is stored at -6-k
  // for castling kill1 is the Rook

  gain = center[m[3]][m[2]] - center[m[1]][m[0]];     // centralization bonus
  if(type <= maxPromote && pieceValue[type] < 100) {  // pawn push bonus (TODO: non-FIDE Pawns)
    gain += (strength[1-side] > 3520 ? 1280 : 4800 - strength[1-side]) >> 7;
    if(m[0] < 2 || (board[m[1]][m[0]-2] & 511) != type) gain -= 10;
    if(m[0] > files-3 || (board[m[1]][m[0]+2] & 511) != type) gain -= 10;
  }
  if(royal[type]) {
    var side = stm >> 10;
    if(pieceValue[type] == strength[side]) gain *= 3; // bare King
    if(strength[1-side] > 1100) gain = -20; else {    // discourage King moves
      var dx1 = m[0] - last[2], dx2 = m[2] - last[2];
      var dy1 = m[1] - last[3], dy2 = m[3] - last[3];
      gain += (dx1*dx1 + dy1*dy1 - dx2*dx2 - dy2*dy2)*5; // award approaching action
    }
  }

  board[m[1]][m[0]] = shadow[m[1]][m[0]]; // remove piece, to recognize unload on start square
  m[-6] = piece;
  for(var k=1; k<m[-2]; k++) { // regular and locust victims
    var x = m[2*k], y = m[2*k+1];
    m[-k-6] = victim = board[y][x];               // record old board state
    var vtype = victim & 511;
    if(vtype) {
      if(victim-stm & 1024) {                     // capture foe, regular or locust
        gain += pieceValue[vtype] + center[y][x]; // count captured value
        var mask = 0x1552551;                     // unit bits of property counters
        if(last[-19]) { // anti-trade in force; test for counter-strike and temp royalty
          var s = 1 + 50*y + x;
          if(last[-19] == -s) specials |= 0x1000; // hit temp royalty, set win flag
          if(1 + 50*last[1] + last[0] == s ||     // capture of mover is never counter-strike
             vtype != last[-19]) mask &= ~0x40;   // and against other type neither, so remove from test
        }
        specials += royalness[vtype] & mask;      // collect royalty and ironhood of victims
      } else if(!m[-5]) {                         // pre-initialized unload must be castling
        gain -= pieceValue[vtype] + center[y][x]; // otherwise count self-destruction value
      }
    } else if(k > 1) { // empty locust square: unload
      if(m[-5] == undefined) {                    // is not pre-initialized
        m[-5] = m[-7];                            // unload victim (perhaps fall-back on locust victim?)
        gain = center[y][x] - center[m[3]][m[2]]; // award its displacement
      } else gain = (m[-5] || m[-2]>4 ? 50 : 30); // castling R deposit or auto-gating
  } }
  board[m[1]][m[0]] = piece;

  if(m[-1] & 256) m[-5] = m[-1], m[-1] = 0, m[-18] = -1;
  if(m[-1]) gain += pieceValue[m[-1]&511] - pieceValue[piece&511]; // promotion gain

  if(specials) { // special eval for game-terminating moves
    a = royalness[type];
//document.getElementById("statuss").innerHTML += 'gain='+gain+' s='+specials.toString(16)+' a='+a.toString(16)+' t='+type+' thr=' + threshold+' pv='+pieceValue[type]+'<br>';
    if(specials & 0x30 ||                  // iron victim
       specials & 0xC0 && last[-19] > 0 || // temp iron
       specials & a & 0x3FE000)            // rel iron
      gain = -15000;                       // this was all illegal no matter what
    else { // not instantly forbidden
      m[-20] = specials & 0xC0F;           // royalty decrement
      m[-18] = 0;                          // always define both royalty updates!
      if(m[-5] & 256) m[-5] -= 256, m[-18] = -1;
      if(specials & 0x1000 ||              // captured temp royal
         royalCount[stm?0:1] < m[-20])     // or exterminated royalty
        return m[-3] = 15000;              // instant win
      if(specials & a & 0x300 &&           // protected iron
        gain < pieceValue[type] + threshold) // and not enough gain on the side to justify it
        m[-19] = -(1 + 50*m[3] + m[2]);    // sets temp royal in antiTrade for next ply
      else if(specials & ~a & 0xC0)        // special taken by non-special triggers counter-strike rule
        m[-19] = victim & 511;             // sets temp iron in antiTrade for next ply
    }
  }

  if(m[-1]) { // promotion
    var a = royalness[type] & 0x401;
    type  = royalness[m[-1]&511] & 0x401;
    if(a != type) { // involves royalty change
      if(!m[-20]) m[-20] = 0; // kludge: define m[-20] to force royalty update
      m[-18] = a - type;      // royalty loss due to promotion
      if(royalCount[stm?1:0] <= m[-18]) gain = -14999; // it killed us
      return m[-3] = gain;
    }
  } else m[-1] = piece;

  if(royalCount[stm?1:0] == 0) {       // we are bare
    if(m[-18] && m[-18]<0) return m[-3] = gain;
    a = royalCount[stm?0:1];
    if(!m[-20] && !a) gain = -20000;   // he was already too, kludge to flag draw
    else if(a > (m[-20] ? m[-20] : 0)) // we did not manage to bare him
      gain = -14999;                   // so we lose
  }

  return m[-3] = gain;
}

function NewMakeMove(m) {
  var k, victim, x = m[0], y = m[1], type = m[-6] & 511;
  board[y][x] = shadow[y][x]; // remove piece
  for(k=1; k<m[-2]; k++) {    // apply other mutations specified in move
    x = m[2*k]; y = m[2*k+1];
    if(m[-6-k]) victim = board[y][x], board[y][x] = shadow[y][x]; // clear any pieces,
    else board[y][x] = (m[-5] ? m[-5] : victim);                  // or 'unload' on empty
  }
  board[m[3]][m[2]] = m[-1] | 512; // place (promoted) piece
  if(m[-20] != undefined) {
    var color = m[-6] >> 10 & 1;
    royalCount[1-color] -= m[-20];     // update opponent's royalty
    royalCount[  color] -= m[-18];     // update own royalty
  }
  if(!imitator[type]) imi = type;
}

function UnMake(m) {
  for(var k=m[-2]; k-->0;) board[m[2*k+1]][m[2*k]] = m[-k-6];
  if(m[-20] != undefined) {
    var color = m[-6] >> 10 & 1;
    royalCount[1-color] += m[-20];     // update opponent's royalty
    royalCount[  color] += m[-18];     // update own royalty
  }
}

function AlphaBeta(stm, alpha, beta, eval, last, preLast, depth, borrow, ply) {
  var iter, nr, d, bestScore, bestNr = -1, firstMove = msp, oldImi = imi, alive = royalCount[stm>>10];
  ep = (last[2*last[-2]] != undefined ? last : 0);  // broadcast e.p. squares
  throughCheck = 0;
  if(ep && ep[-7]&511) { // capturing e.p. (assume sliding royal)
    NewInner(ep[2], ep[3], first[ep[-7]&511], ~0); msp = firstMove; // moves of 'ghost' of victim 
    if(throughCheck) return 15000; // could e.p. capture royal
  }
  bMap = new Array();
  if(preLast) for(nr=0; nr<preLast[-2]; nr++) bMap[2048*preLast[2*nr+1]+preLast[2*nr]] = 2; // squares mutated by us (discovery, plunder)
  for(nr=0; nr<last[-2]; nr++) bMap[2048*last[2*nr+1]+last[2*nr]] = 1; // squares mutated by opponent (recapt, pins)
  if(borrow > 0) bMap[2048*last[3]+last[2]] = 4; // mark recapture square
  GenAll(stm); nodes++;
  if(throughCheck) { msp = firstMove; return 15000; }
  d = depth & 1 | 2; // depth (2 or 3) from where we double IID step to end exactly at requested depth
  for(iter=-1; iter<=depth; iter+=(iter>=d?2:1)) {
    var m, repDep, score, t, val, cost, debt, hashNr = bestNr, ialpha = alpha;
    bestScore = -15000;
    if((iter & ~3) == 0 && alive) {// QS (and not Kingless)
      bestScore = eval;
      if(eval > ialpha) {          // stand pat works
        if(eval >= beta) continue; // cuts off QS iteration
        ialpha = eval;             // use it as score base
    } }
    for(var i=firstMove-(hashNr>=0); i<msp; i++) { // move loop: start one early for hash move
      if(i == hashNr) continue;                            // don't search hash move twice
      m = moveStack[nr = i<firstMove?bestNr:i];            // pre-first = hash move
      t = moveType[nr];
      if((iter & ~1) == 0 && !t) continue;                 // not a new move, don't bother
      score = (m[-3] != undefined ? m[-3] : ScoreMove(m, last)); // calculate eval gain
//if(ply<2)document.getElementById("statuss").innerHTML += '  ' + i + '. ' + m[-20] + ' ' + score + ' ' + bestScore + ' ' + royalCount[0].toString(16) + ' ' + royalCount[1].toString(16) + '<br>';
      if(score > 14000) score = 15000, iter = depth;       // captured royal, kludge to terminate IID
      else if(score <= -14999) {                           // losing termination
        if(score == -15000) continue;                      // illegal, skip
        if(score == -20000) score = 0;                     // instant draw
      } else if(t & 4 && borrow) {                         // we speculatively captured a piece that turns out to be protected
        if(depth >= borrow) depth -= borrow, borrow = 0;   // we should not have done that so cheaply            
        else depth = 0, score = 14000;                     // or in fact could not afford it at all; make the attempt fail low
      } else if(iter >= 0) { // recurse
        val = pieceValue[m[-6]&511];
        if(score > 52) { // move is capture
          if(!t) cost = 2, debt = 0;                       // non-new always 2
          else if(score > val + 40) cost = debt = 0;       // new LxH capts always free
          else {                                           // but for ExE or HxL hope it is unprotected
            cost = (t & 1 ? 0 : 1);                        // recapt tentatively free, plunder still costs 1
            debt = (score < val - 40 ? 2: 1) - cost;       // but if protected raise this to 2 for HxL and 1 for ExE
          }
        } else {
          cost = (t ? 2 : 4); debt = (t ? 0 : -2);         // noncapts cost 2, but non-new get LMR'ed 2 more
        }
        if(cost > iter) score = ialpha; else {             // disarm what we cannot afford
          NewMakeMove(m);
          score = -AlphaBeta(stm^1024, -beta, -ialpha, -eval-score, m, last, iter - cost, debt, ply+1);
          UnMake(m); imi = oldImi;
        }
      } else score -= pieceValue[board[m[1]][m[0]]&512]>>5;// MVV/LVA
//if(!ply||last[1]==6&&last[0]==0&&ply==1/*||last[0]==4&&last[2]==3&&last[3]==9&&ply==2||last[2]==10&&last[3]==10&&ply==3*/)document.getElementById("statuss").innerHTML += ply+':'+iter +  ': ' + i + '. (' + m + ') ' + royalCount[0] + ' ' + royalCount[1] + ' (' + last + ') t='+t+' b='+borrow+' c='+cost+' d='+debt+' s = ' + score + ' best = ' + bestScore + ' i = ' + bestNr + '<br>';
      if(score > bestScore) {
        bestScore = score; bestNr= nr;
        if(score > ialpha) {
          ialpha = score;
          if(score >= (iter < 0 ? 14000 : beta)) break;      // beta cutoff (in MVV iter only on K-capt)
    } } }
    if(borrow < 0 && ialpha < beta && iter == depth)         // parent move scores > alpha at nominal depth
      depth -= borrow, borrow = 0;                           // undo the LMR
  }
  if(bestScore == -15000) { // mate
    bestNr = -1, bestScore += ply;
    if(ply < 10 && AlphaBeta(stm^1024, 14999, 15000, -eval, last, last, 0, 0, ply+1) != 15000 // in check?
       && stalemate == 0) bestScore = 0, bestNr = -2;                    // stalemate
  }
  msp = firstMove; // cleanup move stack
  return (!ply ? bestNr + (bestScore == 14999 ? 10000 : 0) : bestScore); // kludge to flag checkmate in root
}

function SetupCodes() {
  for(var i=-(files + 1024*ranks); i<files+1024*ranks; i++) captCodes[i] = 0;
  for(i=2; i<ranks; i++) {
    captCodes[1024*i] = captCodes[-1024*i] = 32; // vR
    captCodes[1024*i+1] = captCodes[-1024*i-1] = captCodes[1024*i-1] = captCodes[-1024*i+1] = 256;
  }
  for(i=2; i<files; i++) {
    captCodes[i] = captCodes[-i] = 64; // sR
    captCodes[i+1024] = captCodes[-i-1024] = captCodes[i-1024] = captCodes[-i+1+1024] = 256;
  }
  for(i=2; i<ranks || i<files; i++) {
    captCodes[1023*i] = captCodes[-1023*i] = captCodes[1025*i] = captCodes[-1025*i] = 128; // B
  }
  captCodes[1024] = 1; captCode[-1024] = 2; captCodes[1] = captCodes[-1] = 4;    // W
  captCodes[1023] = captCodes[1025] = 8; captCode[-1023] = captCode[-1025] = 16; // F

}

var points, wDens, bDens;
var bb = new Array();
var bb2 = new Array();

function Count(ff, fr, tf, tr) {
  var victim = board[tr][tf];
  var p = 0, rk = 1 << tf;
  for(var k=0; k<kills; k++) {
    var x=killX[k], y=killY[k];
    var victim2 = board[y][x], rk2 = 1<<x;
    if(victim2 & 1024) {
      if(!(bb2[y] & rk2) && bb[y] & rk2) points1--; // upgrade direct capture
      bb[y] &= ~rk2;
      points3++; if(!(bb2[y] & rk2)) lSqrs++;
      if(victim) doubles++;
      bb2[y] |= rk2;
    }
  }
  if(bb[tr] & rk) return; // do not count doubles
  if(victim & 1024) points1++;
  if(!victim) {
    points2++;
    if(fr < ranks - promoZone && tr >= ranks - promoZone) zonePts++;
  }
  bb[tr] |= rk; // mark square as done
}

function RandomSetup(density) {
  // set up random position
  wDens = 0; bDens = 0;
  board = new Array();
  for(var r=0; r<ranks; r++) {
    if(board[r] == undefined) board[r] = new Array();
    for(var f=0; f<files; f++) {
      var x = Math.random();
      var t = Math.random()*nType + 1;
      board[r][f] = (x >= density ? 0 : x >= 0.5*density ? 1024+512+t : 512+t);
      if(board[r][f] & 1024) bDens++; else if(board[r][f]) wDens++;
      if(density == 0) board[r][f] = -Math.floor(((2*r+1-ranks)*(2*r+1-ranks) + (2*f+1-files)*(2*f+1-files))/4);
    }
  }
  wDens /= files*ranks; bDens /= files*ranks;
}

var gauged = -1;

function GaugeValues() { // determine piece values from mobility
  if(gauged == active) return;
  var sum = new Array(), sum2 = new Array(), n = new Array(), z = new Array();
  var b = board;
  gauged = active;
  Emit = Count; imi = 0;
  RandomSetup(0);
  center = board;
  for(var piece=1; piece<=nType; piece++)
    sum[piece] = sum2[piece] = n[piece] = z[piece] = code[piece] = 0;
  for(var k=0; k<10; k++) {
    RandomSetup(0.25);
    for(var piece=1; piece<=nType; piece++) {
      for(var r=0; r<ranks; r++) for(var f=0; f<files; f++) {
        if(board[r][f]) continue;
        for(var i=0; i<ranks; i++) bb[i] = bb2[i] = 0;
        board[r][f] = piece;
        points1 = points2 = points3 = lSqrs = doubles = zonePts = 0;
        var frac = 1.5 - (r + 0.5)/ranks;
        n[piece]++; NewGen(f, r);
        if(points3) points1 += (0.1*lSqrs*points3/(lSqrs + 0.33*points3) + 0*doubles)/bDens;
        points1 /= bDens; points2 /= (1 - wDens - bDens);
        points1 *= 0.67; points2 *= 0.33;
        points1 += points2;
        sum[piece] += points1*frac; sum2[piece] += points1*points1*frac;
        z[piece] += zonePts*frac/(1 - wDens - bDens);
        board[r][f] = 0;
      }
    }
  }
  var ease = 0, best = 0, worst = 100000;
  for(var piece=1; piece<=nType; piece++) {
    sum[piece] /= n[piece]; ease += (piece <= maxPromote ? z[piece] /= n[piece] : 0);
    var s = Math.sqrt(sum2[piece]/n[piece] - sum[piece]*sum[piece]);
    sum[piece] += 0.3*s;
    baseValue[piece] = Math.floor(sum[piece]*(33+sum[piece]*0.7));
  }
  ease /= maxPromote;
  for(var piece=1; piece<=nType; piece++) {
    if(piece <= maxPromote) {
      var promo = piece + promoOffset;
      if(promo > nType) promo = promoOffset + 1;
      f = baseValue[promo]/baseValue[piece]; n[piece] = promo;
      if(f*z[piece] > best && z[piece] > ease) best = f*z[piece]; // most profitable promotion gain of easy promoters
      if(f*z[piece] < worst && z[piece] > ease) worst = f*z[piece];
    }
  }
  for(var piece=1; piece<=nType; piece++) {
    pieceValue[piece] = baseValue[piece];
    if(promoOffset && piece <= maxPromote) {
      if(z[piece] < ease) f = 0.1;
      else if(best == worst) f = 0.2;
      else f = 0.1 + 0.8*(z[piece]*baseValue[n[piece]]/baseValue[piece] - worst)/(best - worst);
      pieceValue[piece] += Math.floor(f*(baseValue[n[piece]] - baseValue[piece]));
    }
    if(royalness[piece] & 0x30) pieceValue[piece] *= 2;
    if(royal[piece] && !extinction) pieceValue[piece] = 15000;
  }
  pieceValue[0] = pieceValue[250] = baseValue[0] = baseValue[250] = 0; // empty square
  threshold = (thresholdPiece ? pieceValue[thresholdPiece] + 30 : 15000);
  board = b;
}

function RefineValues() {
  for(var p=1; p<=nType; p++) if(imitator[p]) {
    var sum=0, sum2=0, sum3 = 0;
    for(var r=0; r<ranks; r++) for(var f=0; f<files; f++) {
      var v = baseValue[board[r][f]&511];
      sum += v; sum2 += v*v; sum3 += v*v*v;
    }
    sum2 /= sum; sum3 /= sum;
    pieceValue[p] = Math.floor(sum2 - 0.3 * Math.sqrt(sum3 - sum2*sum2));
  }
}

function CountRoyals() {
  royalCount[0] = royalCount[1] = 1; // threshold for living
  strength[0] = strength[1] = 0;
  for(var r=0; r<ranks; r++) for(var f=0; f<files; f++) {
    var piece = board[r][f], type = piece & 511, side = piece >> 10 & 1;
    royalCount[side] += royalness[type] & 0x401;
    strength[side] += baseValue[type];
  }
  for(var t=1, r=0; t<=nType; t++) r += royalness[t] & 0x401;
  if(!r) return 0; // no royals; done
  for(f=0; f<2; f++) {
    if(r >= 0x400) { // true royals participate
      royalCount[f] -= 0x400;                         // leaves 1 when we have a single royal
      if(!(extinction & f+1)) royalCount[f] &= 0x3FF; // for absolute royalty, ignore any spares
      if(r & 0x3FF) royalCount[f]--;                  // with baring, just having the one is a delayed loss
    }
  }
  return (royalCount[1] < 0) - (royalCount[0] < 0);
}

function AutoReply(side, x, y) {
  var last = [hx1, hy1, hx2, hy2]; // latest move
  last[-2] = 2;
  if(royalness[captPiece&511] & ~royalness[movedPiece&511] & 0xC0) last[-19] = captPiece & 511;
  if(autoGate && !(movedPiece & 512) && (hy1 == 1 || hy1 == ranks-2)) {
    var gate = (hy1 == 1 ? 0 : ranks-1);
    if((board[gate][hx1] & 511) != 250) {
     ShowMessage('Please put gated piece on board', 'red');
     return;
  } }
  promoTest = (typeof(WeirdPromotion) == 'function');
  GaugeValues();
  RefineValues();
  msp = nodes = 0; imi = imitatedType & 511;
  if(flagX >= 0) { // e.p. rights exist
    var dx = (5*(lastX - flagX) + 8) >> 4;
    var dy = (5*(lastY - flagY) + 8) >> 4;
    last[4] = flagX + dx; last[5] = flagY + dy;
    last[-5] = 0;
  }
//document.getElementById("statuss").innerHTML = 'last = ' + last + '<br>';
  if(CountRoyals()) {
  }
  if(royalCount[side?1:0] < 0) { ShowMessage('It seems I have already lost!', 'red'); return; }
  GenAll(side);
  if(msp) {
    var i = Math.floor(Math.random() * msp);
    var eval = (strength[0] - strength[1])*(side ? -1 : 1);
//document.getElementById("statuss").innerHTML += 'eval = '+ eval + ' ('+strength[0]+'/'+strength[1]+')<br>';
    var n = AlphaBeta(side, -15000, 15000, eval, last, null, 2*depth+2, 0, 0);
    if(n == -1) { ShowMessage('*** I resign! ***', 'red'); return; }
    if(n == -2) { ShowMessage('*** stalemate ***', 'red'); return; }
    if(n > 9999) ShowMessage('*** checkmate! ***', 'red'), n -= 10000;
    var m = moveStack[n];
    if(!(m[-1] - board[m[1]][m[0]] & 511)) m[-1] = 0;
    if(m[-2] == 2 && m[4] != undefined) m[4] = 100; // old system to request e.p.-rights creation
    if(m[4] != undefined) {
//document.getElementById("statuss").innerHTML += 'last = ' + last + ' promo=' + m[-1] + ' (' + m + ') unload=' + m[-5] + '<br>';
         if(autoGate && (m[5] == 0 || m[5] == ranks-1)) {
            if(m[-2] == 6) Move(m[0], m[1], m[2], m[3], m[8], m[9], m[-1]);
            else           Move(m[0], m[1], m[2], m[3], -1, -1, m[-1]);
            Move(m[4], m[5], m[6], m[7], -1, -1, 0);
         } else Move(m[0], m[1], m[2], m[3], m[4], m[5], m[-1]);
    } else Move(m[00], m[1], m[2], m[3], -1, -1, m[-1]);
    if(m[-3] == 15000) ShowMessage('*** I win! ***', 'red');
  }
//document.getElementById("statuss").innerHTML = 'score = ' + m[-3] + ' ' + nodes + ' nodes';
}

function AiMove() {
  AutoReply(~movedPiece & 1024, movedX, movedY);
}

function NewGame() {
  Seek(0); gameLen = 0; document.getElementById("aiGame" + active).innerHTML = '';
  StartPos(); Shuffle(0); Display();
  movedPiece = 1024; movedX = -1; movedY = -1;
}

function Plies(n) {
  depth += n; if(depth > 4) depth = 4; if(depth < 2) depth = 2;
  document.getElementById("ply" + active).innerHTML = depth + ' ply';
}

function DisplayMV(nr) {
  var t, p = document.getElementById('moval' + nr);
  var moval = (p.innerHTML == 'move');
  GaugeValues(); RefineValues();
  p.innerHTML = (moval ? 'estimated value' : 'move');
  for(var i=1; i<=nType; i++) {
    if(moval) t = pieceValue[i] /* + ' ' + baseValue[i] */ + (royalness[i] ? ' (' + royalness[i].toString(16) + ')' : '')
    else t = moves[i]; 
    p = document.getElementById('m' + nr + 'x' + i);
    if(p) p.innerHTML = t;
  }
}

var spare = new Array();
var spareInit = 0;
var touched = 0, lastReq = 0;
var oldMessage = '&nbsp;';

function ShowMoves2(dnr, p, x, y) {
  var q = p;
  var msg;
  SwitchDiag(dnr);
  if(!touched) oldMessage = document.getElementById('message' + dnr).innerHTML;
  if(touched == p && promoOffset && p <= maxPromote) {
    p += promoOffset; if(p > nType) p = promoOffset + 1;
    msg = 'Moves of promoted ' + names[q] + ' (' + names[p] + '):';
  } else msg = 'Moves of ' + names[p & 511] + (p & 512 ? ' (non-virgin)' : '') + ':';
  if(p == 0 || touched == p || touched == p + promoOffset || touched == promoOffset + 1 && p + promoOffset > nType && q == lastReq) {
    touched = emulateHover = 0; ShowMessage(oldMessage);
    Display(); // back to game
    if(xx >= 0) GenMoves(hx1, hy1, (ex >= 0 ? 2 : 1)); // re-apply highlights
    return;
  }
  touched = p; lastReq = q; emulateHover = 1;
  for(var i=0; i<ranks; i++) {
    if(i <= spareInit) { spare[i] = new Array(); spareInit = i + 1; }
    for(var j=0; j<files; j++) { spare[i][j] = board[i][j] & ~(x == -2 ? 3<<28 : 0); board[i][j] = 0; }
  }
  var midX = files >> 1, midY = ranks-1 >> 1, sav1 = hx1, sav2 = hx2, savDark = dark, savLight = light, savEx = ex, savEy = ey, locust=0;
  board[midY][midX] = p; hx1 = -1; hx2 = -1; dark = light = '#FFFFFF';
  if(x >= 0) board[y][x] = 1025, ex = x, ey = y, locust = spare[y][x] >> 29 & 1; 
  Display(); dark = savDark; light = savLight;
  GenMoves(midX, midY, 1+16 + locust);
  if(locust) touched = 0, Highlight(x, y, "#00FFFF"), touched = p;
  ex = savEx, ey = savEy;
  ShowMessage(msg + (promoOffset && p <= maxPromote ? '    <span style="background:#80FF80">Click again to show promoted piece</span>' : ''));
  for(var i=0; i<ranks; i++) {
    for(var j=0; j<files; j++) board[i][j] = spare[i][j];
  }
  hx1 = sav1; hx2 = sav2;
  var desc = document.getElementById('m0');
  p &= 511;
  if(promoOffset && p <= maxPromote && desc) {
    var prom = p + promoOffset;
    if(prom > nType) prom = promoOffset + 1;
    document.getElementById('mp').innerHTML = 'promotes to';
    desc.innerHTML = names[prom];
  }
}

function ShowMoves(p) {
  ShowMoves2(0, p, -2, 0);
}

var printable = 0;

function ShowAll(dnr) {
  var b = document.getElementById("board" + dnr);
  var p = document.getElementById("printversion" + dnr);
  var i, t = '';
  if(p == null) return;
  SwitchDiag(dnr);
  if(printable) { printable = 0; p.innerHTML = ''; return; }
  for(i=1; i<=nType; i++) { // collect move diagrams of all pieces
    var msg = (promoOffset && i <= maxPromote ? ' (promotes to ' + names[i + promoOffset > nType ? promoOffset+1 : i+promoOffset] + ')' : '');
    msg = '<p>moves of ' + names[i] + msg + ':</p>';
    ShowMoves2(active, i, -2, 0);
    table = b.innerHTML;
    t += msg + '<table cellpadding="0" style="border-collapse:collapse;">' + table + '</table>';
  }
  ShowMoves2(active, 0, -2, 0);
  p.innerHTML = t; printable = 1;
}

var lastPiece = 2;

function DefinePiece(n) {
  if(n == 0) { if(touched) ShowMoves(Promoted(touched)); return; }
  if(n > 0) lastPiece = n;
  var entry, v, k = 0;
  entry = document.getElementById('definition')
  if(entry) { v = entry.value; entry.value = ''; if(v != '') moves[lastPiece] = v; k++; }
  entry = document.getElementById('newname')
  if(entry) { v = entry.value; entry.value = ''; if(v != '') names[lastPiece] = v; k++; }
  entry = document.getElementById('newid')
  if(entry) { v = entry.value; entry.value = ''; if(v != '') ids[lastPiece] = v; k++; }
  if(!k) return;
  BError(''); BetzaCompile();
  document.getElementById('m' + active + 'x' + lastPiece).style.background = "#80FF80";
}

function OpenDiv(id) {
  visible = document.getElementById(id).style.display;
  document.getElementById(id).style.display = (visible == "none" ? "inherit" : "none");
  return visible != 'none';
}

