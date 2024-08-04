class AABB:
    def __init__(self, min_x, min_y, max_x, max_y):
        self.min_x = min_x
        self.min_y = min_y
        self.max_x = max_x
        self.max_y = max_y

def get_contact_points(aabb1, aabb2):
    if (aabb1.max_x < aabb2.min_x or aabb1.min_x > aabb2.max_x or
        aabb1.max_y < aabb2.min_y or aabb1.min_y > aabb2.max_y):
        return []  # No intersection

    overlap_min_x = max(aabb1.min_x, aabb2.min_x)
    overlap_min_y = max(aabb1.min_y, aabb2.min_y)
    overlap_max_x = min(aabb1.max_x, aabb2.max_x)
    overlap_max_y = min(aabb1.max_y, aabb2.max_y)

    contact_points = []

    # Only add points from edges that are actually touching
    if overlap_min_x == aabb1.min_x or overlap_min_x == aabb2.min_x:
        contact_points.append((overlap_min_x, overlap_min_y))
        contact_points.append((overlap_min_x, overlap_max_y))

    if overlap_max_x == aabb1.max_x or overlap_max_x == aabb2.max_x:
        contact_points.append((overlap_max_x, overlap_min_y))
        contact_points.append((overlap_max_x, overlap_max_y))

    # Remove duplicate points and limit to 2 points max
    contact_points = list(set(contact_points))[:2]

    return contact_points

# Example usage:
aabb1 = AABB(0, 0, 2, 2)
aabb2 = AABB(1.5, 1.5, 3, 3)

contact_points = get_contact_points(aabb1, aabb2)
print(contact_points)