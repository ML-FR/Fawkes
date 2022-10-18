(deffunction goal-tree-assert-rl (?class $?fact-addresses)
	(bind ?id (sym-cat RL- ?class - (gensym*)))
	(bind ?goal (assert (goal (id ?id) (class ?class) (sub-type RL-OF-SUBGOALS))))
	(printout t "Added RL root goal: "?goal " class " ?class " id " ?id crlf)
	(foreach ?f ?fact-addresses
		(goal-tree-update-child ?f ?id (+ 1 (- (length$ ?fact-addresses) ?f-index)))
		(printout t "Add child " ?f-index ": " ?f  crlf crlf))
	(return ?goal)
)

; #  Goal Creation
(defrule goal-reasoner-parent-create
	(domain-loaded)
	(not (goal))
	(not (test-performed))
  (domain-facts-loaded)
  (skiller-control (acquired TRUE))
	=>

	(goal-tree-assert-run-one TESTGOAL-PARENT

	 (goal-tree-assert-run-all AUTOMATIC-SUBGOAL
		;(goal-tree-assert-run-parallel AUTOMATIC-SUBGOAL NONE
		 ;(assert (goal (id (gensym*)) (class BLOCKS)))
		;(assert (goal (id (gensym*)) (class TOWER-C1) (params buttom a top c)))
		 ;(assert (goal (id (gensym*)) (class TOWER-C2) (params blocks (create$ b d e))))
		;)
		(goal-tree-assert-rl RL
		   (assert (goal (id (sym-cat TOWER-C1- (gensym*))) (class TOWER-C1) (params buttom a top c)))
		   (assert (goal (id (sym-cat TOWER-C1- (gensym*))) (class TOWER-C1) (params buttom b top d)))
		   (assert (goal (id (sym-cat TOWER-C1- (gensym*))) (class TOWER-C1) (params buttom e top d)))
		   ;(assert (goal (id (gensym*)) (class TOWER-C2) (params blocks (create$ b d e))))
		   ;(assert (goal (id (sym-cat RL-TEST- (gensym*))) (class RL)))
		   ;(assert (goal (id (sym-cat FINALLY-SUCCEED- (gensym*))) (class PRINT)))
		)
	 )
	)

	; This is just to make sure we formulate the goal only once.
	; In an actual domain this would be more sophisticated.
	(assert (test-performed))
)


; #  Goal Selection
; We can choose one or more goals for expansion, e.g., calling
; a planner to determine the required steps.
(defrule goal-reasoner-parent-select
	?g <- (goal (id ?goal-id) (class TESTGOAL-PARENT) (mode FORMULATED))
	=>
	(modify ?g (mode SELECTED))
)

; #  Parent Goal Expansion
(defrule goal-reasoner-parent-expand
	?g <- (goal (id ?goal-id) (class TESTGOAL-PARENT) (mode SELECTED))
	=>
	(modify ?g (mode EXPANDED))
)

; # Parent Goal evaluation
(defrule goal-reasoner-goal-evaluate
	?g <- (goal (id ?goal-id) (class TESTGOAL-PARENT) (mode FINISHED))
	=>
  (modify ?g (mode EVALUATED))
)

; # Parent Goal Clean up
(defrule goal-reasoner-goal-cleanup-completed
	?g <- (goal (id ?goal-id) (class TESTGOAL-PARENT) (mode EVALUATED))
	=>
   (printout t "completed clean up - or other testgoal parent evaluated" crlf crlf)
  (modify ?g (mode RETRACTED))
)

(defrule goal-reasoner-expanded
	?g <- (goal (id ?goal-id) (class TALK|PRINT|BLOCKS|TOWER-C1|TOWER-C2) (mode SELECTED))
	(plan (goal-id ?goal-id))
	=>
  (modify ?g (mode EXPANDED))
)
