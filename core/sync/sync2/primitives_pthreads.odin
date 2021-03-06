//+build linux, darwin, freebsd
//+private
package sync2

when #config(ODIN_SYNC_USE_PTHREADS, false) {

import "intrinsics"
import "core:time"
import "core:sys/unix"

_Mutex_State :: enum i32 {
	Unlocked = 0,
	Locked   = 1,
	Waiting  = 2,
}
_Mutex :: struct {
	pthread_mutex: unix.pthread_mutex_t,
}

_mutex_lock :: proc(m: ^Mutex) {
	err := unix.pthread_mutex_lock(&m.impl.pthread_mutex);
	assert(err == 0);
}

_mutex_unlock :: proc(m: ^Mutex) {
	err := unix.pthread_mutex_unlock(&m.impl.pthread_mutex);
	assert(err == 0);
}

_mutex_try_lock :: proc(m: ^Mutex) -> bool {
	err := unix.pthread_mutex_trylock(&m.impl.pthread_mutex);
	return err == 0;
}



RW_Mutex_State :: distinct uint;
RW_Mutex_State_Half_Width :: size_of(RW_Mutex_State)*8/2;
RW_Mutex_State_Is_Writing :: RW_Mutex_State(1);
RW_Mutex_State_Writer     :: RW_Mutex_State(1)<<1;
RW_Mutex_State_Reader     :: RW_Mutex_State(1)<<RW_Mutex_State_Half_Width;

RW_Mutex_State_Writer_Mask :: RW_Mutex_State(1<<(RW_Mutex_State_Half_Width-1) - 1) << 1;
RW_Mutex_State_Reader_Mask :: RW_Mutex_State(1<<(RW_Mutex_State_Half_Width-1) - 1) << RW_Mutex_State_Half_Width;


_RW_Mutex :: struct {
	// NOTE(bill): pthread_rwlock_t cannot be used since pthread_rwlock_destroy is required on some platforms
	// TODO(bill): Can we determine which platforms exactly?
	state: RW_Mutex_State,
	mutex: Mutex,
	sema:  Sema,
}

_rw_mutex_lock :: proc(rw: ^RW_Mutex) {
	_ = intrinsics.atomic_add(&rw.impl.state, RW_Mutex_State_Writer);
	mutex_lock(&rw.impl.mutex);

	state := intrinsics.atomic_or(&rw.impl.state, RW_Mutex_State_Writer);
	if state & RW_Mutex_State_Reader_Mask != 0 {
		sema_wait(&rw.impl.sema);
	}
}

_rw_mutex_unlock :: proc(rw: ^RW_Mutex) {
	_ = intrinsics.atomic_and(&rw.impl.state, ~RW_Mutex_State_Is_Writing);
	mutex_unlock(&rw.impl.mutex);
}

_rw_mutex_try_lock :: proc(rw: ^RW_Mutex) -> bool {
	if mutex_try_lock(&rw.impl.mutex) {
		state := intrinsics.atomic_load(&rw.impl.state);
		if state & RW_Mutex_State_Reader_Mask == 0 {
			_ = intrinsics.atomic_or(&rw.impl.state, RW_Mutex_State_Is_Writing);
			return true;
		}

		mutex_unlock(&rw.impl.mutex);
	}
	return false;
}

_rw_mutex_shared_lock :: proc(rw: ^RW_Mutex) {
	state := intrinsics.atomic_load(&rw.impl.state);
	for state & (RW_Mutex_State_Is_Writing|RW_Mutex_State_Writer_Mask) == 0 {
		ok: bool;
		state, ok = intrinsics.atomic_cxchgweak(&rw.impl.state, state, state + RW_Mutex_State_Reader);
		if ok {
			return;
		}
	}

	mutex_lock(&rw.impl.mutex);
	_ = intrinsics.atomic_add(&rw.impl.state, RW_Mutex_State_Reader);
	mutex_unlock(&rw.impl.mutex);
}

_rw_mutex_shared_unlock :: proc(rw: ^RW_Mutex) {
	state := intrinsics.atomic_sub(&rw.impl.state, RW_Mutex_State_Reader);

	if (state & RW_Mutex_State_Reader_Mask == RW_Mutex_State_Reader) &&
	   (state & RW_Mutex_State_Is_Writing != 0) {
	   	sema_post(&rw.impl.sema);
	}
}

_rw_mutex_try_shared_lock :: proc(rw: ^RW_Mutex) -> bool {
	state := intrinsics.atomic_load(&rw.impl.state);
	if state & (RW_Mutex_State_Is_Writing|RW_Mutex_State_Writer_Mask) == 0 {
		_, ok := intrinsics.atomic_cxchg(&rw.impl.state, state, state + RW_Mutex_State_Reader);
		if ok {
			return true;
		}
	}
	if mutex_try_lock(&rw.impl.mutex) {
		_ = intrinsics.atomic_add(&rw.impl.state, RW_Mutex_State_Reader);
		mutex_unlock(&rw.impl.mutex);
		return true;
	}

	return false;
}

_Cond :: struct {
	pthread_cond: unix.pthread_cond_t,
}

_cond_wait :: proc(c: ^Cond, m: ^Mutex) {
	err := unix.pthread_cond_wait(&c.impl.pthread_cond, &m.impl.pthread_mutex);
	assert(err == 0);
}

_cond_wait_with_timeout :: proc(c: ^Cond, m: ^Mutex, timeout: time.Duration) -> bool {
	ns := time.duration_nanoseconds(timeout);
	timeout_timespec := &time.TimeSpec{
		tv_sec  = ns / 1e9,
		tv_nsec = ns % 1e9,
	};
	err := unix.pthread_cond_timedwait(&c.impl.pthread_cond, &m.impl.pthread_mutex, timeout_timespec);
	// TODO(bill):
	return err == 0;
}

_cond_signal :: proc(c: ^Cond) {
	err := unix.pthread_cond_signal(&c.impl.pthread_cond);
	assert(err == 0);
}

_cond_broadcast :: proc(c: ^Cond) {
	err := unix.pthread_cond_broadcast(&c.impl.pthread_cond);
	assert(err == 0);
}


} // ODIN_SYNC_USE_PTHREADS
