/**
 * @file voice_manager.h
 * @brief Polyphonic voice management system for synthesizer
 * @author Your Name
 * @date 2025
 *
 * This module provides a voice allocation system for managing polyphonic
 * synthesis with up to 6 voices. It handles voice assignment, voice stealing,
 * and note tracking for a polyphonic synthesizer.
 */

#ifndef VOICE_MANAGER_H
#define VOICE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Voice Manager Configuration
 * @{
 */
#define MAX_VOICES 6       /**< Maximum number of simultaneous voices */
#define INVALID_VOICE 0xFF /**< Invalid voice index indicator */
#define INVALID_NOTE 0xFF  /**< Invalid MIDI note indicator */




/** @} */

/**
 * @brief Voice structure containing note assignment and state information
 */
typedef struct {
    uint8_t note;       /**< Assigned MIDI note (INVALID_NOTE if free) */
    uint32_t timestamp; /**< Voice allocation timestamp for age tracking */
    bool active;        /**< Voice active state */
    bool in_release_stage; /**< Flag for release stage (for ADSR) */
} voice_t;

/**
 * @brief Voice manager system structure
 */
typedef struct {
    voice_t voices[MAX_VOICES]; /**< Array of voice structures */
    uint32_t global_timestamp;  /**< Global timestamp counter */
} voice_manager_t;

extern voice_manager_t g_voice_manager;

/**
 * @defgroup Voice Manager API
 * @{
 */

/**
 * @brief Initialize the voice management system
 *
 * Sets all voices to inactive state and resets the global timestamp.
 * Must be called before using any other voice manager functions.
 */
void voice_manager_init(void);

/**
 * @brief Get the next available free voice
 *
 * @return Voice index (0-5) if available, INVALID_VOICE if all voices are in
 * use
 */
uint8_t voice_manager_get_free_voice(void);

/**
 * @brief Find the oldest active voice (for voice stealing)
 *
 * Used when all voices are allocated and a new note needs to be played.
 * Returns the voice that was allocated earliest.
 *
 * @return Index of the oldest voice (0-5)
 */
uint8_t voice_manager_get_oldest_voice(void);

/**
 * @brief Find which voice is assigned to a specific MIDI note
 *
 * @param note MIDI note number to search for
 * @return Voice index (0-5) if note is assigned, INVALID_VOICE if note not
 * found
 */
uint8_t voice_manager_get_voice_by_note(uint8_t note);

/**
 * @brief Assign a MIDI note to a specific voice
 *
 * @param voice_idx Voice index (0-5)
 * @param note MIDI note number to assign
 * @return true if assignment successful, false if invalid voice index
 */
bool voice_manager_assign_note(uint8_t voice_idx, uint8_t note);

/**
 * @brief Release a specific voice by index
 *
 * @param voice_idx Voice index to release (0-5)
 * @return true if release successful, false if invalid voice index
 */
bool voice_manager_release_voice(uint8_t voice_idx);

/**
 * @brief Release the voice assigned to a specific MIDI note
 *
 * @param note MIDI note number to release
 * @return true if note was found and released, false if note not assigned
 */
bool voice_manager_release_note(uint8_t note);

/**
 * @brief Get the number of currently active voices
 *
 * @return Number of active voices (0-6)
 */
uint8_t voice_manager_get_active_count(void);

/**
 * @brief Check if a specific voice is active
 *
 * @param voice_idx Voice index to check (0-5)
 * @return true if voice is active, false otherwise
 */
bool voice_manager_is_voice_active(uint8_t voice_idx);

bool voice_manager_is_voice_in_release_stage(uint8_t voice_idx);
void voice_manager_set_voice_in_release_stage(uint8_t voice_idx, bool state);


/**
 * @brief Get the MIDI note assigned to a specific voice
 *
 * @param voice_idx Voice index (0-5)
 * @return MIDI note number or INVALID_NOTE if voice is inactive
 */
uint8_t voice_manager_get_voice_note(uint8_t voice_idx);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* VOICE_MANAGER_H */