

#include <cassert>
#include <iostream>

#include <slower.h>

#include "subscription.h"



Subscriptions::Subscriptions() {
  subscriptions.resize(128);
}
  
void Subscriptions::add(  const ShortName& name, const int mask, const SlowerRemote& remote ) {
  assert( mask == 16 );
  ShortName group = name;
  group.part[0] &= 0xFFFFffffFFFF0000l;

  //std::cerr << std::hex << group.part[1] << "-" <<  group.part[0] << std::dec << std::endl;
  
  auto mapPtr =  subscriptions[mask].find( group );
  if ( mapPtr == subscriptions[mask].end() ) {
    std::set<SlowerRemote> list;
    list.insert( remote );
    std::pair<ShortName,std::set<SlowerRemote>> pair;
    pair = make_pair( group , list );
    subscriptions[mask].insert( pair );
  }
  else {
    std::set<SlowerRemote>& list = mapPtr->second;
    if ( list.find( remote ) == list.end() ) {
      list.insert( remote ); 
    }
  }
}
  
void Subscriptions::remove(  const ShortName& name, const int mask, const SlowerRemote& remote ) {
  assert( mask == 16 );
  ShortName group = name;
  group.part[0] &= 0xFFFFffffFFFF0000l;

  auto mapPtr = subscriptions[mask].find( group );
  if ( mapPtr != subscriptions[mask].end() ) {
    std::set<SlowerRemote>& list = mapPtr->second;
    if ( list.find( remote ) == list.end() ) {
      list.erase( remote ); 
    }
  }
}
  
std::list<SlowerRemote> Subscriptions::find(  const ShortName& name  ) {
  const int mask=16; // TODO - work over all levels 
  ShortName group = name;
  group.part[0] &= 0xFFFFffffFFFF0000l;
  std::list<SlowerRemote> ret;

  auto mapPtr = subscriptions[mask].find( group );
  if ( mapPtr != subscriptions[mask].end() ) {
    std::set<SlowerRemote>& list = mapPtr->second;
    for( const SlowerRemote& remote : list ) {
      SlowerRemote dest = remote;
      ret.push_back( dest );
    }
  }
  return ret;
}


