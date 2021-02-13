;; Run with > emacs -batch -l ./scripts/emacs/load-srefactor.el <total processes> <process index>

;; Load srefactor
;; NOTE - we have modified it in a few ways to better support the GOAL LISP dialect
(add-to-list 'load-path "./third-party/srefactor")
(require 'srefactor)
(require 'srefactor-lisp)
;; Load benchmark.el for benchmarking
(add-to-list 'load-path "./third-party/benchmark")
(require 'benchmark)

(defvar total-processes (string-to-number (nth 3 command-line-args)))
(defvar process-index (string-to-number (nth 4 command-line-args)))

;; Load all *.gs, *.gd, and *.gc files
(defun load-goal-files ()
  (directory-files-recursively "." ".*\\.g[cds]$"))

(defun batch-files (files batch-size batch-index)
  (nth batch-index
       (seq-partition files
		      (/ (length files)
			 batch-size))))

(defvar goal-files (load-goal-files))

;; ---- Test
;; (pp (batch-files goal-files total-processes process-index))

(defun format-file (file)
  ;; Insert file into current buffer
  (find-file file)
  ;; Swap Quasiquote Instances, Swap any commas not followed by whitespace
  (while (re-search-forward "\\(,\\)\\([^[:space:]]+\\)" nil t)
    (replace-match "___" nil nil nil 1))
  ;; Format with srefactor
  (srefactor-lisp-format-buffer)
  ;; Swap comma placeholders back to commas
  (goto-char 0)
  (while (re-search-forward "\\(___\\)\\([^[:space:]]+\\)" nil t)
    (replace-match "," nil nil nil 1))
  ;; Save current buffer to file
  (save-buffer)
  (kill-buffer)
  (message (format "Formatted - %s" file))
  )

;; ---- TEST
(defvar test "./goal_src/goal-lib.gc")
(pp (format "Formatting Took - %.2f seconds"
	    (benchmark-elapse (format-file test))))
;; ----

;; (defvar files-to-format (batch-files goal-files total-processes process-index))

;; (message (format "[EMACS-%d] Formatting %d goal files (*.gs, *.gc, *.gd)"
;; 		 process-index
;; 		 (length files-to-format)))

;; (pp (format "[EMACS-%d] Formatting Took - %.2f seconds"
;; 	    process-index
;; 	    (benchmark-elapse (mapc 'format-file files-to-format))))

