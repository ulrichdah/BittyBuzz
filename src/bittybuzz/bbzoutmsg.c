#include "bbzoutmsg.h"

/****************************************/
/****************************************/

void bbzoutmsg_queue_construct() {
    bbzringbuf_construct(&vm->outmsgs.queue, (uint8_t*)vm->outmsgs.buf, sizeof(bbzmsg_t), BBZOUTMSG_QUEUE_CAP+1);
}

/****************************************/
/****************************************/

void bbzoutmsg_queue_destruct() {
    bbzringbuf_clear(&vm->outmsgs.queue);
}

/****************************************/
/****************************************/

uint16_t bbzoutmsg_queue_size() {
    return bbzringbuf_size(&vm->outmsgs.queue);
}

/****************************************/
/****************************************/

void bbzoutmsg_queue_append_broadcast(bbzheap_idx_t topic, bbzheap_idx_t value) {
    /* Make a new BROADCAST message */
    bbzmsg_t* m = ((bbzmsg_t*)bbzringbuf_at(&vm->outmsgs.queue, vm->outmsgs.queue.dataend + vm->outmsgs.queue.capacity));
    m->bc.type = BBZMSG_BROADCAST;
    m->bc.rid = vm->robot;
    m->bc.topic = topic;
    m->bc.value = value;
    if (bbzringbuf_full(&vm->outmsgs.queue)) {
        // If full, replace the message with the lowest priority (the last of the queue) with the new one.
        *((bbzmsg_t*)bbzringbuf_rawat(&vm->outmsgs.queue, vm->outmsgs.queue.dataend - (uint8_t)1 + vm->outmsgs.queue.capacity)) = *m;
    }
    else {
        // If not full, push the message at the end of the queue.
        bbzringbuf_makeslot(&vm->outmsgs.queue);
    }
    bbzmsg_sort_priority(&vm->outmsgs.queue);
}

/****************************************/
/****************************************/

void bbzoutmsg_queue_append_swarm_chunk(bbzrobot_id_t rid, bbzswarmlist_t swarms, bbzlamport_t lamport) {
    /* Make a new SWARM_CHUNK message */
    bbzmsg_t* m = ((bbzmsg_t*)bbzringbuf_at(&vm->outmsgs.queue, vm->outmsgs.queue.dataend + vm->outmsgs.queue.capacity));
    m->sw.type = BBZMSG_SWARM_CHUNK;
    m->sw.rid = rid;
    m->sw.lamport = lamport;
    m->sw.swarms = swarms;
    if (bbzringbuf_full(&vm->outmsgs.queue)) {
        // If full, replace the message with the lowest priority (the last of the queue) with the new one.
        *((bbzmsg_t*)bbzringbuf_at(&vm->outmsgs.queue, vm->outmsgs.queue.dataend - (uint8_t)1 + vm->outmsgs.queue.capacity)) = *m;
    }
    else {
        // If not full, push the message at the end of the queue.
        bbzringbuf_makeslot(&vm->outmsgs.queue);
    }
    bbzmsg_sort_priority(&vm->outmsgs.queue);
}

/****************************************/
/****************************************/

void bbzoutmsg_queue_append_vstig(bbzmsg_payload_type_t type,
                                  bbzrobot_id_t rid,
                                  uint16_t key,
                                  bbzheap_idx_t value,
                                  uint8_t lamport) {
    /* Make a new VSTIG_PUT/VSTIG_QUERY message */
    bbzmsg_t* m = ((bbzmsg_t*)bbzringbuf_rawat(&vm->outmsgs.queue, vm->outmsgs.queue.dataend + vm->outmsgs.queue.capacity));
    m->vs.type = type;
    m->vs.rid = rid;
    m->vs.lamport = lamport;
    m->vs.key = key;
    m->vs.data = value;
    if (bbzringbuf_full(&vm->outmsgs.queue)) {
        // If full, replace the message with the lowest priority (the last of the queue) with the new one.
        *((bbzmsg_t*)bbzringbuf_at(&vm->outmsgs.queue, vm->outmsgs.queue.dataend - (uint8_t)1 + vm->outmsgs.queue.capacity)) = *m;
    }
    else {
        // If not full, push the message at the end of the queue.
        bbzringbuf_makeslot(&vm->outmsgs.queue);
    }
    bbzmsg_sort_priority(&vm->outmsgs.queue);
}

/****************************************/
/****************************************/

void bbzoutmsg_queue_first(bbzmsg_payload_t* buf) {
    bbzmsg_t* msg = (bbzmsg_t*)bbzringbuf_at(&vm->outmsgs.queue, 0);
    bbzmsg_serialize_u8(buf, msg->type);
    switch (msg->type) {
        case BBZMSG_BROADCAST:
            if (bbztype_istable(*bbzheap_obj_at(msg->bc.value))) return;
            bbzmsg_serialize_u16(buf, msg->bc.rid);
            bbzmsg_serialize_u16(buf, msg->bc.topic);
            bbzmsg_serialize_obj(buf, bbzheap_obj_at(msg->bc.value));
            break;
        case BBZMSG_VSTIG_PUT: // fallthrough
        case BBZMSG_VSTIG_QUERY:
            if (bbztype_istable(*bbzheap_obj_at(msg->bc.value))) return;
            bbzmsg_serialize_u16(buf, msg->vs.rid);
            bbzmsg_serialize_u16(buf, msg->vs.key);
            bbzmsg_serialize_obj(buf, bbzheap_obj_at(msg->vs.data));
            bbzmsg_serialize_u8(buf, msg->vs.lamport);
            break;
        case BBZMSG_SWARM_CHUNK:
            bbzmsg_serialize_u16(buf, msg->sw.rid);
            bbzmsg_serialize_u16(buf, msg->sw.lamport);
            bbzmsg_serialize_u8(buf, msg->sw.swarms);
            break;
        default:
            break;
    }
}

/****************************************/
/****************************************/

void bbzoutmsg_queue_next() {
    bbzringbuf_pop(&vm->outmsgs.queue);
#ifdef BBZMSG_POP_NEEDS_SORT
    bbzmsg_sort_priority(&vm->outmsgs.queue);
#endif
}

/****************************************/
/****************************************/
