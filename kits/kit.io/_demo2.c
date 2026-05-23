extern struct input {
int   (*count)(void);                                               // number of players
float (*keyboard)(unsigned device, const char *button_expression);  // device=[0] any/first connected keyboard, [1..N] specific player keyboard
float (*gamepad)(unsigned device, const char *button_expression);   // device=[0] any/first connected gamepad,  [1..N] specific player gamepad
float (*mouse)(unsigned device, const char *button_expression);     // etc.
float (*touch)(unsigned device, const char *button_expression);
} input;

// digital examples

if( input.keyboard(0,"a") );                   // if key A is pressed in this frame
if( input.keyboard(0,"*") );                   // if any key is pressed in this frame
if( input.keyboard(0,"a && b") );              // if both A and B keys are pressed in this frame. chord.
if( input.keyboard(0,"!a") );                  // if key A is not pressed in this frame
if( input.keyboard(0,"idle(c)") );             // if key C is idle (not pressed) in this frame
if( input.keyboard(0,"down(a)") );             // if key A was exactly pressed in this frame
if( input.keyboard(0,"held(b)") );             // if key B has been held during this frame
if( input.keyboard(0,"up(a)") );               // if key A was just released in this frame
if( input.keyboard(0,"clicked(a,ms(100))") );  // if key A was clicked (down,up,down) during last 100 ms
if( input.keyboard(0,"clicked(a,frames(10))") );  // if key A was clicked (down,up,down) during last 10 frames

// analog examples

bool connected = input.gamepad(0, "*");                    // is any input feeding?
float right = input.gamepad(0, "LT.x");                    // read left-thumb, x axis 
bool pressed = input.gamepad(0, "pressed(LT.x,0.3)");      // read activity. 0.3 deadzone. same than if "now(x) > then(x)" expression
float forward = input.gamepad(0,"delta(LT.y,frames(10))"); // how much Y was moved since last 10 frames
float forward1 = input.gamepad(0,"delta(LT.y)");           // how much Y was moved since last frame

// analog output

input.gamepad(0, "rumble(ms(100))")

// other. considering:

input.keyboard(0, "down(a) twice(ms(300))")  // double tap A within 300ms

or...

input.keyboard(0, "repeat(a, 2, ms(300))");

// bindings

input.bind("jump", "keyboard(0,space) || gamepad(0,A)");
float j = input.action("jump");

