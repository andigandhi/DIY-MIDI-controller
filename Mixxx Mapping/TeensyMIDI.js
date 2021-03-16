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
