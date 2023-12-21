#pragma once

#ifndef _SERIALIZATION
#define _SERIALIZATION



#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"
#include "Material.h"

#include<iostream>
#include<fstream>

enum class ObjectType
{
    MATERIAL,
    TRANSFORM,
    RENDER,
    PLAYER,
    WORLD

};

class Serialization
{
public:
	Serialization();

    template <typename T>
    static std::string Serialize(const T& object, ObjectType type)
    {
        rapidjson::Document document;
        document.SetObject();

        if (type == ObjectType::MATERIAL)
        {
            SerializeMaterial(document, object);
        }

        std::ofstream file("jsonTest.json");

        if (!file.is_open()) 
        {
            std::cerr << "Error opening file: " << "jsonTest.json" << std::endl;

        }

        rapidjson::OStreamWrapper osw(file);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

        rapidjson::StringBuffer buffer;
        //rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        file.close();

        return buffer.GetString();
    }


    template <typename T>
    static bool Deserialize(const std::string& jsonString, T& object, ObjectType type)
    {
        rapidjson::Document document;
        document.Parse(jsonString.c_str());

        if (!document.HasParseError()) 
        {
            if (type == ObjectType::MATERIAL)
            {
                DeserializeMaterial(document, object);
            }
            return true;
        }

        return false;
    }


private:

    template <typename T>
    static void SerializeMaterial(rapidjson::Document& document, T& object)
    {
        Material* mat = reinterpret_cast<Material*>(object);

        // Serialize material name
       // rapidjson::Value name;
        //name.SetString(mat->getName().c_str());

       //document.AddMember("Name", name, document.GetAllocator());

        document["name"].SetString((char*)mat->getName().data(), mat->getName().size(), document.GetAllocator());

        int i = 0;

        // Serialize texture paths
        for (auto& it : mat->vTexture)
        {
            std::string path = it.first;

            //rapidjson::Value texturePath;
            //texturePath.SetString(path);

           // document.AddMember("Texture " + std::to_string(i), texturePath, document.GetAllocator());
           // document["myValue"].SetString(s.data(), s.size(), document.GetAllocator());
            i++;
        }

        /*
        // Serialize float values
        for (auto& it : mat->vFloat)
        {
            std::string name = it.first;
            float value = it.second.val;

            rapidjson::Value floatVal;
            floatVal.SetFloat(value);

            document.AddMember(name, floatVal, document.GetAllocator());
        }


        rapidjson::Value colorArray;
        colorArray.SetArray();

        // Serialize color values
        for (auto& it : mat->vColor)
        {
            std::string name = it.first;
            Color3 value = it.second;

            rapidjson::Value comp;

            comp.Set(value.r);
            colorArray.PushBack(comp, document.GetAllocator());

            comp.Set(value.g);
            colorArray.PushBack(comp, document.GetAllocator());

            comp.Set(value.b);
            colorArray.PushBack(comp, document.GetAllocator());

            document.AddMember(name, colorArray, document.GetAllocator());
        }


        rapidjson::Value vec3Array;
        vec3Array.SetArray();

        // Serialize vec3 values
        for (auto& it : mat->vVec3)
        {
            std::string name = it.first;
            glm::vec3 value = it.second.val;

            rapidjson::Value comp;

            comp.Set(value.x);
            colorArray.PushBack(comp, document.GetAllocator());

            comp.Set(value.y);
            colorArray.PushBack(comp, document.GetAllocator());

            comp.Set(value.z);
            colorArray.PushBack(comp, document.GetAllocator());

            document.AddMember(name, vec3Array, document.GetAllocator());
        }

        rapidjson::Value vec2Array;
        vec2Array.SetArray();

        // Set vec2 uniforms
        for (auto& it : mat->vVec2)
        {
            std::string name = it.first;
            glm::vec2 value = it.second.val;

            rapidjson::Value comp;

            comp.Set(value.x);
            colorArray.PushBack(comp, document.GetAllocator());

            comp.Set(value.y);
            colorArray.PushBack(comp, document.GetAllocator());

            document.AddMember(name, vec2Array, document.GetAllocator());

        }
        */
    }



    template <typename T>
    static Material* DeserializeMaterial(rapidjson::Document& document, T& object)
    {
        /*
        const rapidjson::Value& exampleField = document["exampleField"];
        if (exampleField.IsInt()) 
        {
            object.exampleField = exampleField.GetInt();
        }

        const rapidjson::Value& anotherField = document["anotherField"];
        if (anotherField.IsString()) 
        {
            object.anotherField = anotherField.GetString();
        }
        */
    }



};





#endif