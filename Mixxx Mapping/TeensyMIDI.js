var Teensy = {};


Teensy.wheelTurn = function (channel, control, value, status, group) {
    // B: For a control that centers on 0x40 (64):
    var newValue = value - 64;
    
   
    var deckNumber = 0;
	switch (group) {
		case "[Channel1]":
			deckNumber = 1;
			break;
		case "[Channel2]":
			deckNumber = 2;
			break;
		case "[Channel3]":
			deckNumber = 3;
			break;
		case "[Channel4]":
			deckNumber = 4;
			break;
	}
	
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue); // Scratch!
    } else {
        engine.setValue(group, 'jog', newValue); // Pitch bend
    }
}