(require 'find-lisp)

;; input dir
(setq inputDir ".") ; In elisp, dir path should end with a slash
;; get all functions to be searched and put into list
(defun read-lines (filePath)
  (with-temp-buffer
	(insert-file-contents filePath)
	(split-string (buffer-string) "\n" t)))
;; store resulting list in global variable
(setq searchList (read-lines "iup.def"))

;; main function
(defun sym-vis-fix (searchFunc)
  (let (outputBuffer)
	(setq outputBuffer "*my occur output*" )
	(with-output-to-temp-buffer outputBuffer
	  (mapc 'my-process-file (find-lisp-find-files inputDir "\\.h$"))
	  (princ "Done deal!"))))

(defun my-process-file (fPath)
  "Process the file at FPATH …"
  (let (myBuffer p1 p2 (ii 0) searchStr)
	(when (not (string-match "/xx" fPath)) ; exclude some dir
	  (let ((fileChanged-p nil))
		(with-temp-buffer
		  ;; insert code from file into temp buffer
		  (insert-file-contents fPath nil nil nil t)

		  ;; need to get all search keys in here and iterate through each
		  (setq searchStr searchFunc) ; search string here
		  ;; start from first point in buffer
		  (goto-char 1)
		  (while (search-forward searchStr nil t)
			;; go to beginning of function def line
			(forward-word -3)
			(if (not (string-equal (thing-at-point 'word) "IUP_EXPORTI"))
				(forward-word 1)
			  (insert "IUP_EXPORTI "))
			;; go back to cursor start
			(forward-word 3)
			(setq fileChanged-p t)
			;; (setq ii (1+ ii))
			)
		  (when fileChanged-p (write-region 1 (point-max) fPath)))))))


;; apply main function to the function list to get everything started
(mapc 'sym-vis-fix searchList)
