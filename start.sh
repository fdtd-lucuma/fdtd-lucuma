#!/usr/bin/env bash
# shellcheck disable=SC2155

NAME="hello"
DESCRIPTION="A C++ template"
VERSION="0.0.0"
REPOSITORY="otreblan/$NAME"
HOMEPAGE_URL="https://github.com/$REPOSITORY"
YEAR="$(date +%Y)"
USER_NAME="Otreblan"
USER_EMAIL="otreblain@gmail.com"

function esc() {
	printf "%s" "$@" | sed -e 's/[\/&]/\\&/g'
}

while IFS= read -rd "" filename; do
	sed -i "$filename" \
		-e "s/{{ NAME }}/$(esc "$NAME")/g" \
		-e "s/{{ DESCRIPTION }}/$(esc "$DESCRIPTION")/g" \
		-e "s/{{ VERSION }}/$(esc "$VERSION")/g" \
		-e "s/{{ REPOSITORY }}/$(esc "$REPOSITORY")/g" \
		-e "s/{{ HOMEPAGE_URL }}/$(esc "$HOMEPAGE_URL")/g" \
		-e "s/{{ YEAR }}/$(esc "$YEAR")/g" \
		-e "s/{{ USER_NAME }}/$(esc "$USER_NAME")/g" \
		-e "s/{{ USER_EMAIL }}/$(esc "$USER_EMAIL")/g"

	git mv --verbose --force "$filename" "${filename%.in}"
done < <(find . -name "*.in" -print0)
