#ifndef NODE_H
#define NODE_H

template <typename T>
class Node
{
public:
    T data;
    Node<T> *next;
    Node<T> *prev;

    Node(): data{}, next{nullptr}, prev{nullptr}
    {
    }

    Node(T data): data{data}, next{nullptr}, prev{nullptr}
    {
    }

    Node(T data, Node<T> *next, Node<T> *prev): data{data}, next{next}, prev{prev}
    {
    }
};

#endif