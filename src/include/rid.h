#ifndef RID_H
#define RID_H

class RID {
    public:
        RID();
        RID(int page, int slot);
        ~RID();
        
        int getPage() const;
        int getSlot() const;
        
    private:
        int page, slot;
};

#endif