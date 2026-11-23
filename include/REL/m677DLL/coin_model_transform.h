#ifndef REL_SEER_TERROR_COIN_MODEL_TRANSFORM_H
#define REL_SEER_TERROR_COIN_MODEL_TRANSFORM_H

// Advances the coin's spin and places its model at the current position.
template <class Entry>
inline void updateCoinModelTransform(Entry *entry)
{
    entry->rotation.y += 12.0f;
    if (entry->rotation.y >= 360.0f) {
        entry->rotation.y -= 360.0f;
    }
    entry->model->setPosition(&entry->position);
    entry->model->setRotation(&entry->rotation);
}

#endif
