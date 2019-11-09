#ifndef ArmPositionList_hpp
#define ArmPositionList_hpp


struct ArmPosition { 

    float pediestal;
    float lower_arm;
    float upper_arm;
    float gripper;
    int timeIndex;

    ArmPosition(float ped,float la, float ua, float grp, int ti):
      pediestal(ped),lower_arm(la),upper_arm(ua),gripper(grp), timeIndex(ti) {};

    setPosition(float ped,float la, float ua, int grp){
        pediestal = ped;
        lower_arm = la;
        upper_arm = ua;
        gripper =grp;
    }

    bool operator == (const ArmPosition item){
        this == &item;
    };

    ArmPosition& operator = (const ArmPosition item){
        pediestal = item.pediestal;
        lower_arm = item.lower_arm;
        upper_arm = item.upper_arm;
        gripper = item.gripper;
        timeIndex = item.timeIndex;
        return *this;
     };

    
}; 


class ListNode {
  public:
    ArmPosition element;
    ListNode* next;
    ListNode* prev;

    ListNode(ArmPosition element, ListNode* prev, ListNode* next) : element(element)
    {
      this->next = next;
      this->prev = prev;
    };
};


class ArmPositionList  {
  private:
    ArmPosition startingPosition;

    ArmPosition currentPosition;

    int indexPartMovement;

    int length;
    ListNode* head;
    ListNode* tail;
    ListNode* curr;
  public:
    ArmPositionList();
    ~ArmPositionList();
    ArmPosition& getCurrent() ;
    ArmPosition& getPrev();
    ArmPosition& getCurrentPosition();
    ArmPosition& First() const;
    ArmPosition& Last() const;
    int getLength();
   

    void AddPosition(float ped,float la, float ua, float grp, int ti);
    void Append(ArmPosition);
    void DeleteLast();
    void DeleteFirst();
    void DeleteCurrent();
    bool next();
    bool moveToStart();
    bool prev();
    void Delete(ArmPosition&);
    void StartMovement(float ped,float la, float ua, float grp);
    void StartMovement(ArmPosition start);
    void StopMovement();
    bool Search(ArmPosition);
    void Clear();
    void Update(ArmPosition elem);

    bool UpdateCurrentPosition();
    void UpdateCurrentPosition(ArmPosition start, ArmPosition end);

};


ArmPositionList::ArmPositionList(): startingPosition(0.f,0.f,0.f,0.f,0.f), currentPosition(0.f,0.f,0.f,0.f,0.f) {
    length = 0;
    indexPartMovement = 0;
    head = nullptr;
    tail = nullptr;
    curr = nullptr;
}


ArmPositionList::~ArmPositionList() {
    Clear();
}


ArmPosition& ArmPositionList::getCurrent()
{
  return curr->element;
}



ArmPosition& ArmPositionList::getPrev()
{
  if(length == 0 || curr->prev == nullptr){
        return startingPosition;
  }
  return curr->prev->element;
}


ArmPosition& ArmPositionList::First() const
{
  return head->element;
}


ArmPosition& ArmPositionList::Last() const
{
  return tail->element;
}


int ArmPositionList::getLength()
{
  return length;
}

void ArmPositionList::Append(ArmPosition element)
{
    ListNode * node = new ListNode(element, tail, nullptr);

    if(length == 0)
        curr = tail = head = node;
    else {
        tail->next = node;
        tail = node;
    }

    length++;

}

void ArmPositionList::AddPosition(float ped,float la, float ua, float grp, int ti)
{

    ArmPosition* pos = new ArmPosition(ped, la, ua, grp, ti);
    Append(*pos);
}


void ArmPositionList::DeleteLast()
{
    if(length == 0)
      return;
    curr = tail;
    DeleteCurrent();
}


void ArmPositionList::DeleteFirst()
{
    if(length == 0)
      return;
    curr = head;
    DeleteCurrent();
}


bool ArmPositionList::next()
{
    if(length == 0)
        return false;

    if(curr->next == nullptr)
        return false;

    curr = curr->next;
    return true;
}


bool ArmPositionList::moveToStart()
{
    currentPosition = startingPosition;

    indexPartMovement = 0;
    curr = head;
    return length != 0;
}


bool ArmPositionList::prev()
{
    if(length == 0)
        return false;

    if(curr->prev != nullptr)
        return false;

    curr = curr->prev;
    return true;
}


void ArmPositionList::Delete(ArmPosition & elem)
{
    if(Search(elem))
        DeleteCurrent();
}


void ArmPositionList::DeleteCurrent()
{
    if(length == 0)
        return;
    length--;
    ListNode * temp = curr;

    if(temp->prev != nullptr)
        temp->prev->next = temp->next;
    if(temp->next != nullptr)
        temp->next->prev = temp->prev;

    if(length == 0)
        head = curr = tail = nullptr;
    else if(curr == head)
        curr = head = head->next;
    else if(curr == tail)
        curr = tail = tail->prev;
    else
        curr = curr->prev;

     delete temp;
}

void ArmPositionList::StartMovement(float ped,float la, float ua, float grp){
   startingPosition.setPosition(ped, la, ua, grp);
   moveToStart();
};

void ArmPositionList::StartMovement(ArmPosition startPosition){
   startingPosition = startPosition;
   moveToStart();
};


void ArmPositionList::StopMovement(){
    currentPosition = startingPosition;
    curr = tail;
    indexPartMovement = tail == nullptr?0:tail->element.timeIndex;
    
};


ArmPosition& ArmPositionList::getCurrentPosition() {
    return currentPosition;
}

bool  ArmPositionList::UpdateCurrentPosition(){
    if (indexPartMovement >= getCurrent().timeIndex){
        
        if (!next()){ // end of movement
           return false; 
        };
    indexPartMovement=0;
    }

    if (getCurrent() == head->element) {
       UpdateCurrentPosition(startingPosition, getCurrent() );
    } else {
      UpdateCurrentPosition(getPrev(), getCurrent() );
    }
    return true;
}

void ArmPositionList::UpdateCurrentPosition(ArmPosition start, ArmPosition end){
    if (indexPartMovement ==0 ){
        currentPosition = start;
        indexPartMovement++;
    } else if (indexPartMovement  < end.timeIndex){
        currentPosition.pediestal = (end.pediestal - start.pediestal) * indexPartMovement /(float)end.timeIndex +start.pediestal;  
        currentPosition.lower_arm = (end.lower_arm - start.lower_arm) * indexPartMovement /(float)end.timeIndex +start.lower_arm;  
        currentPosition.upper_arm = (end.upper_arm - start.upper_arm) * indexPartMovement /(float)end.timeIndex +start.upper_arm;  
        currentPosition.gripper = (end.gripper - start.gripper) * indexPartMovement /(float)end.timeIndex +start.gripper;    
        indexPartMovement++;
    } else {
        currentPosition = end;    
    };
   
}


bool ArmPositionList::Search(ArmPosition elem)
{
    if(length == 0)
        return false;
    if(moveToStart())
        do {
            if(curr->element == elem)
                return true;
        } while (next());
    return false;
}


void ArmPositionList::Update(ArmPosition elem)
{
    if(Search(elem))
        curr->element = elem;
}


void ArmPositionList::Clear()
{
    if(length == 0)
        return;
    ListNode * temp = head;

    while(temp != nullptr)
    {
        head = head->next;
        delete temp;
        temp = head;
    }

    head = curr = tail = nullptr;
}

#endif
