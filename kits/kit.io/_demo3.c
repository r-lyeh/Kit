extern struct keyboard { // provide low-level direct access to keyboard[0]
	float (*up)(const char *vk);     // !0 if  then && !now. eg, float released = keyboard.up("F12");
	float (*down)(const char *vk);   // !0 if !then &&  now
	float (*idle)(const char *vk);   // !0 if !then && !now
	float (*held)(const char *vk);   // !0 if  then &&  now
	float (*tapped)(const char *vk); // !0 if vk went idle>down>held>up>idle cycle
	float (*repeat)(const char *vk, unsigned delay_frames); // !0 if vk is repeated (after >= delay_frames)
	float (*any)(void);              // !0 if any key is pressed
} keyboard;

extern struct mouse { // provide low-level direct access to mouse[0]
	float (*read)(const char *vk);  // buttons: 'l'eft,'r'ight,'m'iddle,x'1',x'2' / axis: 'x','y','w'heel
	float (*delta)(const char *vk); // diff with previous frame
	float (*up)(const char *vk);
	float (*down)(const char *vk);
	float (*idle)(const char *vk);
	float (*held)(const char *vk);
	float (*tapped)(const char *vk);
	float (*repeat)(const char *vk, unsigned delay_frames);
	float (*any)(void); // if any button/axis was pressed
} mouse;

extern struct gamepad { // provide low-level direct access to gamepads[..]
	// buttons abxy / 'n','s','w','e', dpad '<'/'>'/'^'/'v', LB/RB 'L1'/'R1', 'L2/R2', 'L3/R3'
	// axes    'lx'/'ly' left,'rx'/'ry' right
	float (*connected)(int gid);
	float (*read)(int gid, const char *vk);
	float (*delta)(int gid, const char *vk);
	float (*up)(int gid, const char *vk);
	float (*down)(int gid, const char *vk);
	float (*idle)(int gid, const char *vk);
	float (*held)(int gid, const char *vk);
	float (*tapped)(int gid, const char *vk);
	float (*repeat)(int gid, const char *vk, unsigned delay_frames);
	float (*any)(int gid); // if any button/axis was pressed
} gamepad;

extern struct touch { // provide low-level access to touch input (mobile/trackpad)
    // id = finger index [0..N-1]. use touch.count() to know how many are active.
    int   (*count)(void);              // number of active touch points this frame
    float (*x)(int id);                // normalised position 0..1 (screen space)
    float (*y)(int id);                // normalised position 0..1
    float (*pressure)(int id);         // 0..1, if device supports it, else 1 when touching
    float (*delta_x)(int id);          // movement since last frame
    float (*delta_y)(int id);
    float (*down)(int id);             // !0 if finger just touched down this frame
    float (*up)(int id);               // !0 if finger just lifted this frame
    float (*held)(int id);             // !0 if finger is currently in contact
    float (*tapped)(int id);           // !0 if finger completed a tap cycle
    float (*any)(void);                // !0 if any finger is touching
    // gestures (single-finger)
    float (*swipe_x)(void);            // accumulated x swipe delta this frame across all fingers
    float (*swipe_y)(void);            // accumulated y swipe delta
    // gestures (multi-finger)
    float (*pinch)(void);              // scale factor delta (>1 expanding, <1 contracting)
    float (*rotate)(void);             // rotation delta in degrees this frame
} touch;

extern struct input { // high level actions, via lua expressions
	bool  (*bind)(const char *name, const char *expr); // bool ok = input.bind("jump", "keyboard(0,space) || gamepad(0,A)"). returns false if expr is invalid
	float (*action)(const char *name);                 // bool triggered = !!input.action("jump");
	bool  (*unbind)(const char *name);                 // name or "*" to remove all bindings. wildcards allowed; eg, .unbind("player2.*")

	float (*eval)(const char *expr);                   // float Y = input.eval("gamepad(0, forward)"). returns nan if expr is invalid
} input;

/*
default keyboard<->gamepad bindings
[up] = SDL_SCANCODE_UP;
[down] = SDL_SCANCODE_DOWN;
[left] = SDL_SCANCODE_LEFT;
[right] = SDL_SCANCODE_RIGHT;
[select] = SDL_SCANCODE_SPACE;
[start] = SDL_SCANCODE_RETURN;
[a] = SDL_SCANCODE_A;
[b] = SDL_SCANCODE_B;
[x] = SDL_SCANCODE_X;
[y] = SDL_SCANCODE_Y;
[l1] = SDL_SCANCODE_1;
[l2] = SDL_SCANCODE_2;
[l3] = SDL_SCANCODE_3;
[r1] = SDL_SCANCODE_6;
[r2] = SDL_SCANCODE_5;
[r3] = SDL_SCANCODE_4;
*/