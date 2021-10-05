;---------------------------------------------------------------------------
;  goal-class.clp - CLIPS executive - goal class representation
;
;  Created: Sun Sep 12 20:44:00 2021
;  Copyright  2021  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
;  Licensed under GPLv2+ license, cf. LICENSE file
;---------------------------------------------------------------------------

(deftemplate goal-class
    "
    A description of the properties of a goal class specifying its
    preconditions, effects, parameters and types.
    In the goal lifecycle, goal classes are used to specify the preconditions
    for formulation of goals in a general way. These preconditions are grounded
    automatically similarly to plan actions and subsequently checked for
    satisfcation.
    "
    (slot class (type SYMBOL))
    (slot type (type SYMBOL) (allowed-values ACHIEVE MAINTAIN) (default ACHIEVE))
    (slot sub-type (type SYMBOL))
    (multislot param-names)
    (multislot param-constants)
    (multislot param-quantified)
    (multislot param-types)
    (slot preconditions (type STRING))
    (slot effects (type STRING))
)

(deffunction goal-class-create-grounding
    (?goal-class ?param-types ?param-names ?param-names-left ?param-constants ?param-quantified ?param-values)


    (if (> (length$ ?param-types) 0)
        then
        (if (neq (nth$ 1 ?param-constants) nil)
            then
                (bind ?param-values-new (insert$ ?param-values (+ 1 (length$ ?param-values)) (nth$ 1 ?param-constants)))
                (goal-class-create-grounding ?goal-class (delete$ ?param-types 1 1) (delete$ ?param-names 1 1) ?param-names-left  (delete$ ?param-constants 1 1) ?param-quantified ?param-values-new)
            else
            (if (not (member$ (nth$ 1 ?param-names) ?param-quantified))
                then
                (do-for-all-facts ((?object domain-object)) (eq ?object:type (nth$ 1 ?param-types))
                    (bind ?param-values-new (insert$ ?param-values (+ 1 (length$ ?param-values)) ?object:name))
                    (goal-class-create-grounding ?goal-class (delete$ ?param-types 1 1) (delete$ ?param-names 1 1) ?param-names-left (delete$ ?param-constants 1 1) ?param-quantified ?param-values-new)
                )
                else
                (bind ?param-values-new (insert$ ?param-values (+ 1 (length$ ?param-values)) nil))
                (goal-class-create-grounding ?goal-class (delete$ ?param-types 1 1) (delete$ ?param-names 1 1) ?param-names-left (delete$ ?param-constants 1 1) ?param-quantified ?param-values-new)
            )
        )
        else
        (if (not (any-factp ((?grounding pddl-grounding)) (and (eq ?grounding:formula-root ?goal-class) (eq ?grounding:param-values ?param-values))))
            then
            (bind ?grounding-id (sym-cat "grounding-" ?goal-class "-" (gensym*)))
            (assert (pddl-grounding (param-names ?param-names-left)
                                    (param-values ?param-values)
                                    (formula-root ?goal-class)
                                    (id ?grounding-id)
                    )
            )
        )
    )
)

(defrule goal-class-grounding
    (domain-facts-loaded)
    (goal-class (class ?class-id)
                (param-names $?param-names)
                (param-constants $?param-constants)
                (param-types $?param-types)
                (param-quantified $?param-quantified)
    )

    (pddl-formula (part-of ?class-id) (id ?formula-id))

    (or
        (and
            (domain-object (name ?object) (type ?type&:
                        (and
                            (member$ ?type ?param-types)
                            (eq (nth$ (member$ ?type ?param-types) ?param-constants) nil)
                        ))
            )

            (not
                (and
                    (pddl-grounding (id ?grounding-id) (param-names $?param-names) (param-values $? ?object $?))
                    (grounded-pddl-formula (formula-id ?formula-id) (grounding ?grounding-id))
                )
            )
        )
        (not
            (and
                (pddl-grounding (id ?grounding-id))
                (grounded-pddl-formula (formula-id ?formula-id) (grounding ?grounding-id))
            )
        )
    )
    =>
    (goal-class-create-grounding ?class-id ?param-types ?param-names ?param-names ?param-constants ?param-quantified (create$ ))
)
