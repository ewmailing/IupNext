; Symbol Visibility Fix
; Author: Chris Matzenbach
#lang racket

;; packages
(require racket/path)

;; global vars
(define inputDir ".") 

;; get function list from .def files
(define (get-function-list fle)
  (file->lines fle))

;; removes any empty strings in a list
(define (remove-empty-strings-from-list lst)
  (filter (lambda (x) (not (equal? "" x))) lst))

;; search through file for matching string - case sensitive
;; returns the string if match is found, false otherwise
(define (find-string-in-file fle str)
  (call-with-input-file fle
    (lambda (in)
      (let ([res (string-contains? (port->string in) str)])
                   (if (eq? res #t) str #f)))))

;; file string text -> void
;; finds first match of string in file, then adds text right before it (with spacing)
(define (add-text-to-file fle str txt)
  ;; store file data as a list of strings, with each line as a string
  (let ([file-data (file->lines fle)])
    (for ([e (in-list file-data)])
      (cond
        [(and (string-contains? e str) (regexp-match #px"\\S+\\s+\\S*\\s*\\S*\\w+\\(.*?\\)" e)) (display (~a e "\n"))]
        ;; [(string-contains? e str) (display (~a e "\n"))]
          ;; now need a regex test to make sure it is the definition and not a func call or coment or random mention
          ))))

;; to add data back into file - (display-lines-to-file lines file[string])

;; goes item by item from .def file and searching each file with each iteration
(define (start defFile dir)
  (let ([funcList (remove-empty-strings-from-list (get-function-list defFile))])
    (for ([str funcList])
      (for ([fle (in-directory dir)]
          ;; only loop through items that meet the condition(s) below
          #:when (and
                  ;; check if it exists
                  (file-exists? fle)
                  ;; get file extension, which is either a byte string or false
                  (equal?
                   ;; return false if it's false or convert byte string to string otherwise
                   (if (equal? (path-get-extension fle) #f)
                              #f
                              (bytes->string/utf-8 (path-get-extension fle)))
                   ;; we only want header files, so comparing extension to .h
                   ".h")
                  ;; and make sure to exclude the stupid .cquery dir
                  (not (string-contains? (some-system-path->string fle) ".cquery"))))
        ;; start body of (for) loop
        (if (equal? #t (not (find-string-in-file (some-system-path->string fle) str)))
            #f
            ;; we have a match - need to find line of match
            (add-text-to-file fle str "something")
            )))))

;; searches by file, and sees if there's a match in the list when it's in that file
(define (launch defFile dir)
  (let ([funcList (remove-empty-strings-from-list (get-function-list defFile))])
    ;; open every file in folder and check for match in func list
    (for ([fle (in-directory dir)]
          ;; only loop through items that meet the condition(s) below
          #:when (and
                  ;; check if it exists
                  (file-exists? fle)
                  ;; get file extension, which is either a byte string or false
                  (equal?
                   ;; return false if it's false or convert byte string to string
                   (if (equal? (path-get-extension fle) #f)
                              #f
                              (bytes->string/utf-8 (path-get-extension fle)))
                   ;; we only want header files, so comparing extension to .h
                   ".h")
                  ;; and make sure to exclude the stupid .cquery dir
                  (not (string-contains? (some-system-path->string fle) ".cquery"))))
      ;; start body of (for) loop
      (for ([str funcList])
        (if (equal? #t (not (find-string-in-file (some-system-path->string fle) str)))
            ;; if it's false then do nothing I guess?
            #f
            ;; otherwise it should return the string
            ;; 1. find line in file
            ;; 2. check if IUP_EXPORT is already there
            ;; 3. find beginning of function definition
            ;; 4. insert IUP_EXPORT or IUP_EXPORTI into file
            ;; profit
            (display-lines-to-file (file->lines fle) "test-chris.h"))))))
