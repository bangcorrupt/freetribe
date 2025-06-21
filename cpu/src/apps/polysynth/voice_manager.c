/**
 * @file voice_manager.c
 * @brief Implementation of polyphonic voice management system
 * @author enorrmann
 * @date 2025
 */

#include "voice_manager.h"
#include <string.h>

/* Global voice manager instance */
voice_manager_t g_voice_manager;

void voice_manager_init(void) {
    /* Clear entire structure */
    memset(&g_voice_manager, 0, sizeof(voice_manager_t));
    
    /* Initialize all voices as inactive */
    for (int i = 0; i < MAX_VOICES; i++) {
        g_voice_manager.voices[i].note = INVALID_NOTE;
        g_voice_manager.voices[i].active = false;
        g_voice_manager.voices[i].in_release_stage = false;
        g_voice_manager.voices[i].timestamp = 0;
    }
    
    /* Initialize global timestamp (start at 1 to avoid zero timestamp) */
    g_voice_manager.global_timestamp = 1;
}

uint8_t voice_manager_get_free_voice(void) {
    /* Search for first inactive voice */
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!g_voice_manager.voices[i].active) {
            return i;
        }
    }
    
    /* No free voices available */
    return INVALID_VOICE;
}

uint8_t voice_manager_get_oldest_voice(void) {
    uint8_t oldest_voice = 0;
    uint32_t oldest_timestamp = g_voice_manager.voices[0].timestamp;
    
    /* Find voice with smallest timestamp (oldest) */
    for (int i = 1; i < MAX_VOICES; i++) {
        if (g_voice_manager.voices[i].timestamp < oldest_timestamp) {
            oldest_timestamp = g_voice_manager.voices[i].timestamp;
            oldest_voice = i;
        }
    }
    
    return oldest_voice;
}

uint8_t voice_manager_get_voice_by_note(uint8_t note) {
    /* Search through all active voices for matching note */
    for (int i = 0; i < MAX_VOICES; i++) {
        if (g_voice_manager.voices[i].active && 
            g_voice_manager.voices[i].note == note) {
            return i;
        }
    }
    
    /* Note not found in any voice */
    return INVALID_VOICE;
}

bool voice_manager_assign_note(uint8_t voice_idx, uint8_t note) {
    /* Validate voice index */
    if (voice_idx >= MAX_VOICES) {
        return false;
    }
    
    /* Assign note to voice and mark as active */
    g_voice_manager.voices[voice_idx].note = note;
    g_voice_manager.voices[voice_idx].active = true;
    g_voice_manager.voices[voice_idx].in_release_stage = false;
    g_voice_manager.voices[voice_idx].timestamp = g_voice_manager.global_timestamp++;
    
    return true;
}

bool voice_manager_release_voice(uint8_t voice_idx) {
    /* Validate voice index */
    if (voice_idx >= MAX_VOICES) {
        return false;
    }
    
    /* Reset voice to inactive state */
    g_voice_manager.voices[voice_idx].note = INVALID_NOTE;
    g_voice_manager.voices[voice_idx].active = false;
    g_voice_manager.voices[voice_idx].in_release_stage = false;
    g_voice_manager.voices[voice_idx].timestamp = 0;
    
    return true;
}

bool voice_manager_release_note(uint8_t note) {
    /* Find voice assigned to this note */
    uint8_t voice_idx = voice_manager_get_voice_by_note(note);
    
    if (voice_idx != INVALID_VOICE) {
        return voice_manager_release_voice(voice_idx);
    }
    
    /* Note was not assigned to any voice */
    return false;
}

uint8_t voice_manager_get_active_count(void) {
    uint8_t count = 0;
    
    /* Count all active voices */
    for (int i = 0; i < MAX_VOICES; i++) {
        if (g_voice_manager.voices[i].active) {
            count++;
        }
    }
    
    return count;
}
bool voice_manager_is_voice_in_release_stage(uint8_t voice_idx) {
    /* Validate voice index */
    if (voice_idx >= MAX_VOICES) {
        return false;
    }
    
    return g_voice_manager.voices[voice_idx].in_release_stage && 
           g_voice_manager.voices[voice_idx].active;
}

void voice_manager_set_voice_in_release_stage(uint8_t voice_idx,bool state) {
    /* Validate voice index */
    if (voice_idx < MAX_VOICES) {
        g_voice_manager.voices[voice_idx].in_release_stage = state;
    }
}

bool voice_manager_is_voice_active(uint8_t voice_idx) {
    /* Validate voice index */
    if (voice_idx >= MAX_VOICES) {
        return false;
    }
    
    return g_voice_manager.voices[voice_idx].active;
}

uint8_t voice_manager_get_voice_note(uint8_t voice_idx) {
    /* Validate voice index */
    if (voice_idx >= MAX_VOICES) {
        return INVALID_NOTE;
    }
    
    /* Return note if voice is active, otherwise invalid note */
    if (g_voice_manager.voices[voice_idx].active) {
        return g_voice_manager.voices[voice_idx].note;
    }
    
    return INVALID_NOTE;
}