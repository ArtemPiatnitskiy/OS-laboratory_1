#!/usr/bin/env bash
set -u
PROG=${PROG:-./program}    # путь к вашей программе, можно переопределить ENV: PROG=./program
TESTDIR=${TESTDIR:-tests}  # директория с тестами
TIMEOUT_CMD=${TIMEOUT_CMD:-timeout} # если нет timeout, можно убрать проверку

write_all() { :; } # placeholder (не нужен здесь)

run_one() {
  id="$1"
  in=""
  out_expected=""

  # возможные расширения
  for f in "$TESTDIR"/input_"$id" "$TESTDIR"/input_"$id".txt "$TESTDIR"/input_"$id".in; do
    [ -f "$f" ] && { in="$f"; break; }
  done
  for f in "$TESTDIR"/output_true_"$id" "$TESTDIR"/output_true_"$id".txt "$TESTDIR"/output_true_"$id".out; do
    [ -f "$f" ] && { out_expected="$f"; break; }
  done

  if [ -z "$in" ]; then
    printf "SKIP %s — input not found\n" "$id"
    return 0
  fi
  if [ -z "$out_expected" ]; then
    printf "SKIP %s — expected output not found\n" "$id"
    return 0
  fi

  tmpout="$(mktemp)"
  # запуск с таймаутом (5s), измените при необходимости
  if ! $TIMEOUT_CMD 5s "$PROG" < "$in" > "$tmpout" 2> /dev/null; then
    code=$?
    if [ "$code" -eq 124 ] 2>/dev/null; then
      printf "TIMEOUT %s\n" "$id"
    else
      printf "\x1b[31mERROR %s (exit %d)\n" "$id" "$code"
    fi
    rm -f "$tmpout"
    return 1
  fi

  if diff -u "$out_expected" "$tmpout" > /dev/null; then
    printf "\x1b[32mPASS %s\n" "$id"
    rm -f "$tmpout"
    return 0
  else
    printf "\x1b[0mFAIL %s — see diff below\n" "$id"
    diff -u "$out_expected" "$tmpout" || true
    rm -f "$tmpout"
    return 2
  fi
}

main() {
  if [ "$#" -eq 0 ] || [ "$1" = "all" ]; then
    # найти все input_* в TESTDIR
    shopt -s nullglob
    for f in "$TESTDIR"/input_*; do
      base=$(basename "$f")
      id=${base#input_}
      run_one "$id"
    done
    return
  fi

  # запуск конкретных тестов по аргументам
  for id in "$@"; do
    run_one "$id"
  done
}

main "$@"