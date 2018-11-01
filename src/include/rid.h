#ifndef RID_H
#define RID_H

/*
 * RID is unchangeable.
 * 
 *   The user is not expected to, and also not able to modify
 * an RID instance. Note that an RID instance might become
 * invalid after updating the table, for instance the page might
 * disappear or the slot is empty.
 */
class RID {
    public:
        RID();
        RID(int page, int slot);
        RID(const RID&);
        ~RID();
        
        int getPage() const;
        int getSlot() const;

    private:
        int page;       // page ID
        int slot;       // offset (bytes)
};

#endif