var Teensy = {};


Teensy.wheelTurn = function (channel, control, value, status, group) {
    // B: For a control that centers on 0x40 (64):
    var newValue = value - 64;
    
   
    var deckNumber = 1;
	if (group == "[Channel2]") deckNumber = 2;
	else if (group == "[Channel3]") deckNumber = 3;
	else if (group == "[Channel4]") deckNumber = 4;
	
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue); // Scratch!
    } else {
        engine.setValue(group, 'jog', newValue); // Pitch bend
    }
}

Teensy.setLoopSize = function (channel, control, value, status, group) {
	var size = Math.pow(2, Math.floor(value/10)-5);
	
	engine.setParameter(group, "beatloop_size", size);
	engine.setParameter(group, "beatjump_size", size);
}

var playButtonLED = function (value, group, control) {
	var deckNumber = 0x00;
    if (group === '[Channel2]') deckNumber = 0x01;
	print(value);
	if (value === 1) {
		midi.sendShortMsg(0x90, deckNumber, 0x01);
	} else {
		midi.sendShortMsg(0x80, deckNumber, 0x00);
	}
}

var playConnection1 = engine.makeConnection('[Channel1]', 'play_indicator', playButtonLED);
var playConnection2 = engine.makeConnection('[Channel2]', 'play_indicator', playButtonLED);