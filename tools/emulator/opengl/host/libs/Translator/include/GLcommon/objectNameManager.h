/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef _OBJECT_NAME_MANAGER_H
#define _OBJECT_NAME_MANAGER_H

#include <cutils/threads.h>
#include <map>
#include "SmartPtr.h"

typedef std::map<unsigned int, unsigned int> NamesMap;

enum NamedObjectType {
    VERTEXBUFFER = 0,
    TEXTURE = 1,
    RENDERBUFFER = 2,
    FRAMEBUFFER = 3,
    SHADER = 4,
    PROGRAM = 5,
    NUM_OBJECT_TYPES = 6  // Must be last
};

class ObjectData
{
public:
    virtual ~ObjectData() {}
};
typedef SmartPtr<ObjectData> ObjectDataPtr;

//
// Class NameSpace - this class manages allocations and deletions of objects
//                   from a single "local" namespace (private to context group).
//                   For each allocated object name, a "global" name is
//                   generated as well to be used in the space where all
//                   contexts are shared.
//
//   NOTE: this class does not used by the EGL/GLES layer directly,
//         the EGL/GLES layer creates objects using the ShareGroup class
//         interface (see below).
class NameSpace
{
    friend class ShareGroup;
    friend class GlobalNameSpace;

private:
    explicit NameSpace(NamedObjectType p_type);
    ~NameSpace();

    //
    // genName - creates new object in the namespace and  returns its name.
    //           if p_localName is specified this name will be used.
    //           This function also generate a global name for the object,
    //           the value of the global name can be retrieved using the
    //           getGlobalName function.
    //
    unsigned int genName(unsigned int p_localName = 0, bool genGlobal = true);

    //
    // getGlobalName - returns the global name of an object or 0 if the object
    //                 does not exist.
    //
    unsigned int getGlobalName(unsigned int p_localName);

    //
    // getLocaalName - returns the local name of an object or 0 if the object
    //                 does not exist.
    //
    unsigned int getLocalName(unsigned int p_globalName);

    //
    // deleteName - deletes and object from the namespace as well as its
    //              global name from the global name space.
    //
    void deleteName(unsigned int p_localName);

    //
    // isObject - returns true if the named object exist.
    //
    bool isObject(unsigned int p_localName);

    //
    // replaces an object to map to an existing global object
    //
    void replaceGlobalName(unsigned int p_localName, unsigned int p_globalName);

private:
    unsigned int m_nextName;
    NamesMap m_localToGlobalMap;
    const NamedObjectType m_type;
};

//
// class ShareGroup -
//   That class manages objects of one "local" context share group, typically
//   there will be one inctance of ShareGroup for each user OpenGL context
//   unless the user context share with another user context. In that case they
//   both will share the same ShareGroup instance.
//   calls into that class gets serialized through a lock so it is thread safe.
//
class ShareGroup
{
    friend class ObjectNameManager;
    friend class SmartPtr<ShareGroup>;  // to allow destructing when ShareGroupPtr refcount reaches zero

public:

    //
    // genName - generates new object name and returns its name value.
    //           if p_localName is specified this name will be used.
    //           This function also generates a "global" name for the object
    //           which can be queried using the getGlobalName function.
    //
    unsigned int genName(NamedObjectType p_type, unsigned int p_localName = 0);

    //
    // getGlobalName - retrieves the "global" name of an object or 0 if the
    //                 object does not exist.
    //
    unsigned int getGlobalName(NamedObjectType p_type, unsigned int p_localName);

    //
    // getLocalName - retrieves the "local" name of an object or 0 if the
    //                 object does not exist.
    //
    unsigned int getLocalName(NamedObjectType p_type, unsigned int p_globalName);

    //
    // deleteName - deletes and object from the namespace as well as its
    //              global name from the global name space.
    //
    void deleteName(NamedObjectType p_type, unsigned int p_localName);

    //
    // replaceGlobalName - replaces an object to map to an existing global
    //        named object. (used when creating EGLImage siblings)
    //
    void replaceGlobalName(NamedObjectType p_type, unsigned int p_localName, unsigned int p_globalName);

    //
    // isObject - returns true if the named object exist.
    //
    bool isObject(NamedObjectType p_type, unsigned int p_localName);

    //
    // Assign object global data to a names object
    //
    void setObjectData(NamedObjectType p_type, unsigned int p_localName, ObjectDataPtr data);

    //
    // Retrieve object global data
    //
    ObjectDataPtr getObjectData(NamedObjectType p_type, unsigned int p_localName);

private:
    ShareGroup();
    ~ShareGroup();

private:
    mutex_t m_lock;
    NameSpace *m_nameSpace[NUM_OBJECT_TYPES];
    void *m_objectsData;
};

typedef SmartPtr<ShareGroup> ShareGroupPtr;
typedef std::multimap<void *, ShareGroupPtr> ShareGroupsMap;

//
// ObjectNameManager -
//   This class manages the set of all ShareGroups instances,
//   each ShareGroup instance can be accessed through one or more 'groupName'
//   values. the type of 'groupName' is void *, the intent is that the EGL
//   layer will use the user context handle as the name for its ShareGroup
//   object. Multiple names can be attached to a ShareGroup object to support
//   user context sharing.
//
class ObjectNameManager
{
public:
    ObjectNameManager();
    ~ObjectNameManager();

    //
    // createShareGroup - create a new ShareGroup object and attach it with
    //                    the "name" specified by p_groupName.
    //
    ShareGroupPtr createShareGroup(void *p_groupName);

    //
    // attachShareGroup - find the ShareGroup object attached to the name
    //    specified in p_existingGroupName and attach p_groupName to the same
    //    ShareGroup instance.
    //
    ShareGroupPtr attachShareGroup(void *p_groupName, void *p_existingGroupName);

    //
    // getShareGroup - retreive a ShareGroup object based on its "name"
    //
    ShareGroupPtr getShareGroup(void *p_groupName);

    //
    // deleteShareGroup - deletes the attachment of the p_groupName to its
    //           attached ShareGroup. When the last name of ShareGroup is
    //           deleted the ShareGroup object is destroyed.
    //
    void deleteShareGroup(void *p_groupName);

    //
    //  getGlobalContext() - this function returns a name of an existing
    //                       ShareGroup. The intent is that the EGL layer will
    //                       use that function to get the GL context which each
    //                       new context needs to share with.
    //
    void *getGlobalContext();

private:
    ShareGroupsMap m_groups;
    mutex_t m_lock;
};

#endif
