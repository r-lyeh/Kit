#ifndef MODULE_H
#define MODULE_H

// step,tick
// tick:  1 per frame; so usually means 60 times per second, but also 120 or 144 depending on monitor, user settings, etc.
// step: 60 per second, frame indepedant. you can do 30,20,15,12,10,6,5,4,3,2,1 updates/sec too by updating only once every 2,3,4,5,6,10,12,15,20,30,60 calls.
// load,save
// read,dump
// pull,push
// grab,copy,diff,mend

void on_init   (void (*fn)()); void inits(); // register callback + invoke all callbacks.
void  on_load  (void (*fn)()); void loads(); // register callback + invoke all callbacks.
void   on_play (void (*fn)()); void plays(); // register callback + invoke all callbacks.
void    on_edit(void (*fn)()); void edits(); // register callback + invoke all callbacks.
void    on_step(void (*fn)()); void steps(); // register callback + invoke all callbacks.
void    on_tick(void (*fn)()); void ticks(); // register callback + invoke all callbacks.
void    on_draw(void (*fn)()); void draws(); // register callback + invoke all callbacks.
void    on_swap(void (*fn)()); void swaps(); // register callback + invoke all callbacks.
void   on_stop (void (*fn)()); void stops(); // register callback + invoke all callbacks.
void  on_save  (void (*fn)()); void saves(); // register callback + invoke all callbacks.
void on_quit   (void (*fn)()); void quits(); // register callback + invoke all callbacks.
void on_halt   (void (*fn)()); void halts(); // register callback + invoke all callbacks.
void on_cook   (void (*fn)()); void cooks(); // register callback + invoke all callbacks.

void on_context(void (*fn)()); void contexts(); //< same than above, when graphics contexts are created

#elif KIT_CODE
#pragma once

typedef void (*module_fn)();

array_(module_fn) inits_; void on_init(void (*fn)()) { array_push(inits_, fn); } void inits() { for(int i=0,ii=array_count(inits_);i<ii;++i) inits_[i](); }
array_(module_fn) loads_; void on_load(void (*fn)()) { array_push(loads_, fn); } void loads() { for(int i=0,ii=array_count(loads_);i<ii;++i) loads_[i](); }
array_(module_fn) plays_; void on_play(void (*fn)()) { array_push(plays_, fn); } void plays() { for(int i=0,ii=array_count(plays_);i<ii;++i) plays_[i](); }
array_(module_fn) edits_; void on_edit(void (*fn)()) { array_push(edits_, fn); } void edits() { for(int i=0,ii=array_count(edits_);i<ii;++i) edits_[i](); }
array_(module_fn) steps_; void on_step(void (*fn)()) { array_push(steps_, fn); } void steps() { for(int i=0,ii=array_count(steps_);i<ii;++i) steps_[i](); }
array_(module_fn) ticks_; void on_tick(void (*fn)()) { array_push(ticks_, fn); } void ticks() { for(int i=0,ii=array_count(ticks_);i<ii;++i) ticks_[i](); }
array_(module_fn) draws_; void on_draw(void (*fn)()) { array_push(draws_, fn); } void draws() { for(int i=0,ii=array_count(draws_);i<ii;++i) draws_[i](); }
array_(module_fn) swaps_; void on_swap(void (*fn)()) { array_push(swaps_, fn); } void swaps() { for(int i=0,ii=array_count(swaps_);i<ii;++i) swaps_[i](); }
array_(module_fn) stops_; void on_stop(void (*fn)()) { array_push(stops_, fn); } void stops() { for(int i=0,ii=array_count(stops_);i<ii;++i) stops_[i](); }
array_(module_fn) saves_; void on_save(void (*fn)()) { array_push(saves_, fn); } void saves() { for(int i=0,ii=array_count(saves_);i<ii;++i) saves_[i](); }
array_(module_fn) quits_; void on_quit(void (*fn)()) { array_push(quits_, fn); } void quits() { for(int i=0,ii=array_count(quits_);i<ii;++i) quits_[i](); }
array_(module_fn) halts_; void on_halt(void (*fn)()) { array_push(halts_, fn); } void halts() { for(int i=0,ii=array_count(halts_);i<ii;++i) halts_[i](); }
array_(module_fn) cooks_; void on_cook(void (*fn)()) { array_push(cooks_, fn); } void cooks() { for(int i=0,ii=array_count(cooks_);i<ii;++i) cooks_[i](); }

array_(module_fn) contexts_; void on_context(void (*fn)()) { array_push(contexts_, fn); }
void contexts() { for(int i=0,ii=array_count(contexts_);i<ii;++i) contexts_[i](); }

#if 0 // demo

void my_module_init() {
    puts("init!");
}
void my_module_tick() {
    puts("tick!");
}
void my_module_quit() {
    puts("quit!");
}

AUTORUN {
    on_init(my_module_init);
    on_tick(my_module_tick);
    on_quit(my_module_quit);
}

int main() {
    // do {
    inits();
    loads();
    for(uint64_t frame=0;;++frame) {
        plays();
            edits();
            ticks();
            draws();
            swaps();
        stops();
    }
    saves();
    quits();
    // } while( userquit() ? 0 : app_has_reloaded() );
}

#endif

#endif
