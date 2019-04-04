# Copyright 2018-2019 - Omar Sandoval
# SPDX-License-Identifier: GPL-3.0+

"""
Linked Lists
------------

The ``drgn.helpers.linux.list`` module provides helpers for working with the
doubly-linked list implementations (``struct list_head`` and ``struct
hlist_head``) in :linux:`include/linux/list.h`.
"""

from drgn import container_of


__all__ = [
    'list_empty',
    'list_is_singular',
    'list_for_each',
    'list_for_each_reverse',
    'list_for_each_entry',
    'list_for_each_entry_reverse',
    'hlist_empty',
    'hlist_for_each',
    'hlist_for_each_entry',
]


def list_empty(head):
    """
    .. c:function:: bool list_empty(struct list_head *head)

    Return whether a list is empty.
    """
    head = head.read_()
    return head.next == head


def list_is_singular(head):
    """
    .. c:function:: bool list_is_singular(struct list_head *head)

    Return whether a list has only one element.
    """
    head = head.read_()
    next = head.next
    return next != head and next == head.prev


def list_for_each(head):
    """
    .. c:function:: list_for_each(struct list_head *head)

    Iterate over all of the nodes in a list.

    :return: Iterator of ``struct list_head *`` objects.
    """
    head = head.read_()
    pos = head.next.read_()
    while pos != head:
        yield pos
        pos = pos.next.read_()


def list_for_each_reverse(head):
    """
    .. c:function:: list_for_each_reverse(struct list_head *head)

    Iterate over all of the nodes in a list in reverse order.

    :return: Iterator of ``struct list_head *`` objects.
    """
    head = head.read_()
    pos = head.prev.read_()
    while pos != head:
        yield pos
        pos = pos.prev.read_()


def list_for_each_entry(type, head, member):
    """
    .. c:function:: list_for_each_entry(type, struct list_head *head, member)

    Iterate over all of the entries in a list, given the type of the entry and
    the ``struct list_head`` member in that type.

    :return: Iterator of ``type *`` objects.
    """
    for pos in list_for_each(head):
        yield container_of(pos, type, member)


def list_for_each_entry_reverse(type, head, member):
    """
    .. c:function:: list_for_each_entry_reverse(type, struct list_head *head, member)

    Iterate over all of the entries in a list in reverse order, given the type
    of the entry and the ``struct list_head`` member in that type.

    :return: Iterator of ``type *`` objects.
    """
    for pos in list_for_each_reverse(head):
        yield container_of(pos, type, member)


def hlist_empty(head):
    """
    .. c:function:: bool hlist_empty(struct hlist_head *head)

    Return whether a hash list is empty.
    """
    return not head.first


def hlist_for_each(head):
    """
    .. c:function:: hlist_for_each(struct hlist_head *head)

    Iterate over all of the nodes in a hash list.

    :return: Iterator of ``struct hlist_node *`` objects.
    """
    pos = head.first.read_()
    while pos:
        yield pos
        pos = pos.next.read_()


def hlist_for_each_entry(type, head, member):
    """
    .. c:function:: hlist_for_each_entry(type, struct hlist_head *head, member)

    Iterate over all of the entries in a has list, given the type of the entry
    and the ``struct hlist_node`` member in that type.

    :return: Iterator of ``type *`` objects.
    """
    for pos in hlist_for_each(head):
        yield container_of(pos, type, member)
