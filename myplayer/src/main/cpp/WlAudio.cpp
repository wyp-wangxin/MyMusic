//
// Created by yangw on 2018-2-28.
//

#include "WlAudio.h"



WlAudio::~WlAudio() {

}

WlAudio::WlAudio(WlPlaystatus *wlPlaystatus) {
    this->playstatus = wlPlaystatus;
    queue = new WlQueue(playstatus);
}
